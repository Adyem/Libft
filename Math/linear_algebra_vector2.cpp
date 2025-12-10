#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
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

static void vector2_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
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

int vector2::lock_self(ft_unique_lock<pt_mutex> &guard) const
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

int vector2::lock_pair(const vector2 &first, const vector2 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const vector2 *ordered_first;
    const vector2 *ordered_second;
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
        const vector2 *temporary;

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
        vector2_sleep_backoff();
    }
}

void vector2::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void vector2::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

vector2::vector2(const vector2 &other)
    : _x(0.0), _y(0.0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    *this = other;
    return ;
}

vector2 &vector2::operator=(const vector2 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    vector2_restore_errno(this_guard, entry_errno);
    vector2_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector2::vector2(vector2 &&other)
    : _x(0.0), _y(0.0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector2 &vector2::operator=(vector2 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._x = 0.0;
    other._y = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    other.set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(this_guard, entry_errno);
    vector2_restore_errno(other_guard, entry_errno);
    return (*this);
}

double vector2::get_x() const
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
    vector2_restore_errno(guard, entry_errno);
    return (value);
}

double vector2::get_y() const
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
    vector2_restore_errno(guard, entry_errno);
    return (value);
}

vector2 vector2::add(const vector2 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector2 result;

    entry_errno = ft_errno;
    lock_error = vector2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(this_guard, entry_errno);
    vector2_restore_errno(other_guard, entry_errno);
    return (result);
}

vector2 vector2::subtract(const vector2 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector2 result;

    entry_errno = ft_errno;
    lock_error = vector2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(this_guard, entry_errno);
    vector2_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector2::dot(const vector2 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    double result;

    entry_errno = ft_errno;
    lock_error = vector2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    result = vector2_compute_dot(this->_x, this->_y, other._x, other._y);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(this_guard, entry_errno);
    vector2_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector2::length() const
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
    squared = vector2_compute_dot(this->_x, this->_y, this->_x, this->_y);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(guard, entry_errno);
    return (math_sqrt(squared));
}

vector2 vector2::normalize() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector2 result;
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
    squared_length = this->_x * this->_x + this->_y * this->_y;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        vector2_restore_errno(guard, entry_errno);
        return (result);
    }
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result.set_error_unlocked(FT_ER_SUCCESSS);
    this->set_error_unlocked(FT_ER_SUCCESSS);
    vector2_restore_errno(guard, entry_errno);
    return (result);
}

int vector2::get_error() const
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
    vector2_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *vector2::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}
