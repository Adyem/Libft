#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector4_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static int math_vector4_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int math_vector4_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

void vector4::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

#if defined(__SSE2__)
static double vector4_compute_dot(double first_x, double first_y,
    double first_z, double first_w,
    double second_x, double second_y,
    double second_z, double second_w)
{
    __m128d first_xy;
    __m128d second_xy;
    __m128d product_xy;
    __m128d first_zw;
    __m128d second_zw;
    __m128d product_zw;
    __m128d sum;
    __m128d swapped;
    __m128d total;

    first_xy = _mm_set_pd(first_y, first_x);
    second_xy = _mm_set_pd(second_y, second_x);
    product_xy = _mm_mul_pd(first_xy, second_xy);
    first_zw = _mm_set_pd(first_w, first_z);
    second_zw = _mm_set_pd(second_w, second_z);
    product_zw = _mm_mul_pd(first_zw, second_zw);
    sum = _mm_add_pd(product_xy, product_zw);
    swapped = _mm_shuffle_pd(sum, sum, 0x1);
    total = _mm_add_sd(sum, swapped);
    return (_mm_cvtsd_f64(total));
}
#else
static double vector4_compute_dot(double first_x, double first_y,
    double first_z, double first_w,
    double second_x, double second_y,
    double second_z, double second_w)
{
    return (first_x * second_x + first_y * second_y
        + first_z * second_z + first_w * second_w);
}
#endif

int vector4::lock_pair(const vector4 &first, const vector4 &second,
    const vector4 *&lower, const vector4 *&upper)
{
    const vector4 *ordered_first;
    const vector4 *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (math_vector4_lock_mutex(first._mutex));
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const vector4 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = math_vector4_lock_mutex(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESSS)
        {
            return (lower_error);
        }
        int upper_error = math_vector4_lock_mutex(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            math_vector4_unlock_mutex(lower->_mutex);
            return (upper_error);
        }
        math_vector4_unlock_mutex(lower->_mutex);
        vector4_sleep_backoff();
    }
}

void vector4::unlock_pair(const vector4 *lower, const vector4 *upper)
{
    if (upper != ft_nullptr)
        math_vector4_unlock_mutex(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        math_vector4_unlock_mutex(lower->_mutex);
    return ;
}

vector4::vector4(const vector4 &other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0),
    _operation_errors({{}, {}, 0}), _mutex()
{
    *this = other;
    return ;
}

vector4 &vector4::operator=(const vector4 &other)
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

vector4::vector4(vector4 &&other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0),
    _operation_errors({{}, {}, 0}), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector4 &vector4::operator=(vector4 &&other)
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    this->unlock_pair(lower, upper);
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._w = 0.0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    other.record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

double vector4::get_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_x;
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_y;
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_z() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_z;
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_w() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_w;
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

vector4 vector4::add(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    vector4 result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result._w = this->_w + other._w;
    this->unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

vector4 vector4::subtract(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    vector4 result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result._w = this->_w - other._w;
    this->unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

double vector4::dot(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    double result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    result = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            other._x, other._y, other._z, other._w);
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

double vector4::length() const
{
    int lock_error;
    double squared;
    double result_value;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    squared = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            this->_x, this->_y, this->_z, this->_w);
    result_value = math_sqrt(squared);
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (result_value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result_value);
}

vector4 vector4::normalize() const
{
    int lock_error;
    vector4 result;
    double squared_length;
    double length_value;
    double epsilon;
    int unlock_error;

    lock_error = math_vector4_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    squared_length = this->_x * this->_x + this->_y * this->_y
        + this->_z * this->_z + this->_w * this->_w;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        unlock_error = math_vector4_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            result.record_operation_error(unlock_error);
            this->record_operation_error(unlock_error);
            return (result);
        }
        result.record_operation_error(FT_ERR_INVALID_ARGUMENT);
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result._z = this->_z / length_value;
    result._w = this->_w / length_value;
    unlock_error = math_vector4_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(unlock_error);
        this->record_operation_error(unlock_error);
        return (result);
    }
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

pt_recursive_mutex *vector4::get_mutex_for_validation() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *vector4::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *vector4::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
