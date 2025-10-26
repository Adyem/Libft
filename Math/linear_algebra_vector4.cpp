#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

static void vector4_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void vector4_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int vector4::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
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

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
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

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        vector4_sleep_backoff();
    }
}

void vector4::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void vector4::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

vector4::vector4(const vector4 &other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0), _error_code(ER_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

vector4 &vector4::operator=(const vector4 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
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
    vector4_restore_errno(this_guard, entry_errno);
    vector4_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector4::vector4(vector4 &&other)
    : _x(0.0), _y(0.0), _z(0.0), _w(0.0), _error_code(ER_SUCCESS), _mutex()
{
    *this = ft_move(other);
    return ;
}

vector4 &vector4::operator=(vector4 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
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
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(this_guard, entry_errno);
    vector4_restore_errno(other_guard, entry_errno);
    return (*this);
}

double vector4::get_x() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_x;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (value);
}

double vector4::get_y() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_y;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (value);
}

double vector4::get_z() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_z;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (value);
}

double vector4::get_w() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    value = this->_w;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (value);
}

vector4 vector4::add(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector4 result;

    entry_errno = ft_errno;
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result._w = this->_w + other._w;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(this_guard, entry_errno);
    vector4_restore_errno(other_guard, entry_errno);
    return (result);
}

vector4 vector4::subtract(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    vector4 result;

    entry_errno = ft_errno;
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result._w = this->_w - other._w;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(this_guard, entry_errno);
    vector4_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector4::dot(const vector4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    double result;

    entry_errno = ft_errno;
    lock_error = vector4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    result = this->_x * other._x + this->_y * other._y + this->_z * other._z
        + this->_w * other._w;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(this_guard, entry_errno);
    vector4_restore_errno(other_guard, entry_errno);
    return (result);
}

double vector4::length() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double squared;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0.0);
    }
    squared = this->_x * this->_x + this->_y * this->_y + this->_z * this->_z
        + this->_w * this->_w;
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (math_sqrt(squared));
}

vector4 vector4::normalize() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector4 result;
    double squared_length;
    double length_value;
    double epsilon;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
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
        vector4_restore_errno(guard, entry_errno);
        return (result);
    }
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result._z = this->_z / length_value;
    result._w = this->_w / length_value;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    vector4_restore_errno(guard, entry_errno);
    return (result);
}

int vector4::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    vector4_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *vector4::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}
