#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector2_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static int math_vector2_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int math_vector2_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

void vector2::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

#if defined(__SSE2__)
static double vector2_compute_dot(double first_x, double first_y,
    double second_x, double second_y)
{
    __m128d first_vector;
    __m128d second_vector;
    __m128d product;
    __m128d swapped;
    __m128d sum;

    first_vector = _mm_set_pd(first_y, first_x);
    second_vector = _mm_set_pd(second_y, second_x);
    product = _mm_mul_pd(first_vector, second_vector);
    swapped = _mm_shuffle_pd(product, product, 0x1);
    sum = _mm_add_sd(product, swapped);
    return (_mm_cvtsd_f64(sum));
}
#else
static double vector2_compute_dot(double first_x, double first_y,
    double second_x, double second_y)
{
    return (first_x * second_x + first_y * second_y);
}
#endif

int vector2::lock_pair(const vector2 &first, const vector2 &second,
    const vector2 *&lower, const vector2 *&upper)
{
    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (math_vector2_lock_mutex(first._mutex));
    }
    lower = &first;
    upper = &second;
    if (lower > upper)
    {
        const vector2 *temporary;

        temporary = lower;
        lower = upper;
        upper = temporary;
    }
    while (true)
    {
        int lower_error = math_vector2_lock_mutex(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESSS)
        {
            return (lower_error);
        }
        int upper_error = math_vector2_lock_mutex(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            math_vector2_unlock_mutex(lower->_mutex);
            return (upper_error);
        }
        math_vector2_unlock_mutex(lower->_mutex);
        vector2_sleep_backoff();
    }
}

void vector2::unlock_pair(const vector2 *lower, const vector2 *upper)
{
    if (upper != ft_nullptr)
        math_vector2_unlock_mutex(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        math_vector2_unlock_mutex(lower->_mutex);
    return ;
}

vector2::vector2(const vector2 &other)
    : _x(0.0), _y(0.0),
    _operation_errors({{}, {}, 0}), _mutex()
{
    *this = other;
    return ;
}

vector2 &vector2::operator=(const vector2 &other)
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;

    if (this == &other)
    {
        return (*this);
    }
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

vector2::vector2(vector2 &&other)
    : _x(0.0), _y(0.0),
    _operation_errors({{}, {}, 0}), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector2 &vector2::operator=(vector2 &&other)
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;

    if (this == &other)
    {
        return (*this);
    }
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    other._x = 0.0;
    other._y = 0.0;
    other.record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

double vector2::get_x() const
{
    int lock_error;
    double value;

    lock_error = math_vector2_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_x;
    int unlock_error = math_vector2_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double vector2::get_y() const
{
    int lock_error;
    double value;

    lock_error = math_vector2_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_y;
    int unlock_error = math_vector2_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

vector2 vector2::add(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    vector2 result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    this->unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

vector2 vector2::subtract(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    vector2 result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    this->unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

double vector2::dot(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    double result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    result = vector2_compute_dot(this->_x, this->_y, other._x, other._y);
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

double vector2::length() const
{
    int lock_error;
    int unlock_error;
    double squared;

    lock_error = math_vector2_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    squared = vector2_compute_dot(this->_x, this->_y, this->_x, this->_y);
    unlock_error = math_vector2_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (math_sqrt(squared));
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (math_sqrt(squared));
}

vector2 vector2::normalize() const
{
    int lock_error;
    int unlock_error;
    vector2 result;
    double squared_length;
    double length_value;
    double epsilon;

    lock_error = math_vector2_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    squared_length = this->_x * this->_x + this->_y * this->_y;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        unlock_error = math_vector2_unlock_mutex(this->_mutex);
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
    unlock_error = math_vector2_unlock_mutex(this->_mutex);
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

pt_recursive_mutex *vector2::get_mutex_for_validation() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *vector2::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *vector2::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
