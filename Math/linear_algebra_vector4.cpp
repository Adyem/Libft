#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static thread_local ft_operation_error_stack g_vector4_operation_errors = {{}, {}, 0};

static void vector4_push_error(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(g_vector4_operation_errors, error_code, operation_id);
    return ;
}

static void vector4_sleep_backoff()
{
    pt_thread_sleep(1);
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

int vector4::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

int vector4::lock_pair(const vector4 &first, const vector4 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const vector4 *ordered_first;
    const vector4 *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const vector4 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        vector4_sleep_backoff();
    }
}

void vector4::set_error_unlocked(int error_code) const
{
    vector4_push_error(error_code);
    this->_error_code = error_code;
    return ;
}

void vector4::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

vector4::vector4(const vector4 &other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    *this = other;
    return ;
}

vector4 &vector4::operator=(const vector4 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    return (*this);
}

vector4::vector4(vector4 &&other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector4 &vector4::operator=(vector4 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._w = 0.0;
    other._error_code = FT_ERR_SUCCESSS;
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    return (*this);
}

double vector4::get_x() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    double value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    value = this->_x;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_y() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    double value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    value = this->_y;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_z() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    double value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    value = this->_z;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (value);
}

double vector4::get_w() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    double value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    value = this->_w;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (value);
}

vector4 vector4::add(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    vector4 result;

    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result._w = this->_w + other._w;
    result.set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

vector4 vector4::subtract(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    vector4 result;

    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result._w = this->_w - other._w;
    result.set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

double vector4::dot(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    double result;

    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    result = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            other._x, other._y, other._z, other._w);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

double vector4::length() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    double squared;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0.0);
    }
    squared = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            this->_x, this->_y, this->_z, this->_w);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (math_sqrt(squared));
}

vector4 vector4::normalize() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    vector4 result;
    double squared_length;
    double length_value;
    double epsilon;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    squared_length = this->_x * this->_x + this->_y * this->_y
        + this->_z * this->_z + this->_w * this->_w;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result._z = this->_z / length_value;
    result._w = this->_w / length_value;
    result.set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

int vector4::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (lock_error);
    }
    error_value = this->_error_code;
    return (error_value);
}

const char *vector4::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}
