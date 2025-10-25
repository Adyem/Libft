#include "geometry_aabb.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "geometry_lock_tracker.hpp"

#include <cstddef>
#include <utility>
#include "../Template/move.hpp"

static void geometry_aabb_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

aabb::aabb()
{
    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->_error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

aabb::aabb(double minimum_x, double minimum_y,
        double maximum_x, double maximum_y)
{
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    this->_error_code = ER_SUCCESS;
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

aabb::aabb(const aabb &other)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->_error_code = ER_SUCCESS;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return ;
}

aabb &aabb::operator=(const aabb &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = aabb::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return (*this);
}

aabb::aabb(aabb &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->_error_code = ER_SUCCESS;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->_error_code = other._error_code;
    other._minimum_x = 0.0;
    other._minimum_y = 0.0;
    other._maximum_x = 0.0;
    other._maximum_y = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

aabb &aabb::operator=(aabb &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = aabb::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->_error_code = other._error_code;
    other._minimum_x = 0.0;
    other._minimum_y = 0.0;
    other._maximum_x = 0.0;
    other._maximum_y = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return (*this);
}

aabb::~aabb()
{
    return ;
}

int aabb::set_bounds(double minimum_x, double minimum_y,
        double maximum_x, double maximum_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_minimum(double minimum_x, double minimum_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (minimum_x > this->_maximum_x || minimum_y > this->_maximum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_minimum_x(double minimum_x)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (minimum_x > this->_maximum_x)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_minimum_y(double minimum_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (minimum_y > this->_maximum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_y = minimum_y;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_maximum(double maximum_x, double maximum_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (maximum_x < this->_minimum_x || maximum_y < this->_minimum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_maximum_x(double maximum_x)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (maximum_x < this->_minimum_x)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int aabb::set_maximum_y(double maximum_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (maximum_y < this->_minimum_y)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        geometry_aabb_restore_errno(guard, entry_errno);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_y = maximum_y;
    this->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

double  aabb::get_minimum_x() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_minimum_x;
    const_cast<aabb *>(this)->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (value);
}

double  aabb::get_minimum_y() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_minimum_y;
    const_cast<aabb *>(this)->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (value);
}

double  aabb::get_maximum_x() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_maximum_x;
    const_cast<aabb *>(this)->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (value);
}

double  aabb::get_maximum_y() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_maximum_y;
    const_cast<aabb *>(this)->set_error(ER_SUCCESS);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (value);
}

int aabb::get_error() const
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    geometry_aabb_restore_errno(guard, entry_errno);
    return (error_code);
}

const char  *aabb::get_error_str() const
{
    int entry_errno;
    int error_code;
    const char *error_string;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<aabb *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    geometry_aabb_restore_errno(guard, entry_errno);
    return (error_string);
}

void    aabb::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int aabb::lock_pair(const aabb &first, const aabb &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    pt_mutex &first_mutex = first._mutex;
    pt_mutex &second_mutex = second._mutex;

    return (geometry_lock_tracker_lock_pair(&first, &second,
            first_mutex, second_mutex, first_guard, second_guard));
}
