#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector3_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void vector3_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

#if defined(__SSE2__)
static double vector3_compute_dot(double first_x, double first_y, double first_z,
    double second_x, double second_y, double second_z)
{
    __m128d first_xy;
    __m128d second_xy;
    __m128d product_xy;
    __m128d swapped;
    __m128d sum_xy;
    __m128d first_z_vector;
    __m128d second_z_vector;
    __m128d product_z;
    __m128d total;

    first_xy = _mm_set_pd(first_y, first_x);
    second_xy = _mm_set_pd(second_y, second_x);
    product_xy = _mm_mul_pd(first_xy, second_xy);
    swapped = _mm_shuffle_pd(product_xy, product_xy, 0x1);
    sum_xy = _mm_add_sd(product_xy, swapped);
    first_z_vector = _mm_set_sd(first_z);
    second_z_vector = _mm_set_sd(second_z);
    product_z = _mm_mul_sd(first_z_vector, second_z_vector);
    total = _mm_add_sd(sum_xy, product_z);
    return (_mm_cvtsd_f64(total));
}
#else
static double vector3_compute_dot(double first_x, double first_y, double first_z,
    double second_x, double second_y, double second_z)
{
    return (first_x * second_x + first_y * second_y + first_z * second_z);
}
#endif

int vector3::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

int vector3::lock_pair(const vector3 &first, const vector3 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const vector3 *ordered_first;
    const vector3 *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const vector3 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        vector3_sleep_backoff();
    }
}

void vector3::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void vector3::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

vector3::vector3(const vector3 &other)
    : _x(0.0), _y(0.0), _z(0.0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    *this = other;
    return ;
}

vector3 &vector3::operator=(const vector3 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector3::vector3(vector3 &&other)
    : _x(0.0), _y(0.0), _z(0.0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector3 &vector3::operator=(vector3 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    other.set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (*this);
}

double vector3::get_x() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_x;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(guard, entry_errno);
    return (value);
}

double vector3::get_y() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_y;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(guard, entry_errno);
    return (value);
}

double vector3::get_z() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_z;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(guard, entry_errno);
    return (value);
}

vector3 vector3::add(const vector3 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector3 result;

    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (result);
}

vector3 vector3::subtract(const vector3 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector3 result;

    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector3::dot(const vector3 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    double result;

    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    result = vector3_compute_dot(this->_x, this->_y, this->_z,
            other._x, other._y, other._z);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (result);
}

vector3 vector3::cross(const vector3 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector3 result;

    entry_errno = ft_errno;
    lock_error = vector3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_y * other._z - this->_z * other._y;
    result._y = this->_z * other._x - this->_x * other._z;
    result._z = this->_x * other._y - this->_y * other._x;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(this_guard, entry_errno);
    vector3_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector3::length() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double squared;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    squared = vector3_compute_dot(this->_x, this->_y, this->_z,
            this->_x, this->_y, this->_z);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(guard, entry_errno);
    return (math_sqrt(squared));
}

vector3 vector3::normalize() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector3 result;
    double squared_length;
    double length_value;
    double epsilon;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    squared_length = this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        vector3_restore_errno(guard, entry_errno);
        return (result);
    }
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result._z = this->_z / length_value;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector3_restore_errno(guard, entry_errno);
    return (result);
}

int vector3::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    vector3_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *vector3::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}
