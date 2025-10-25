#include "geometry_circle.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "geometry_lock_tracker.hpp"

#include <cstddef>
#include <utility>
#include "../Template/move.hpp"

static void geometry_circle_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

circle::circle()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->_error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

circle::circle(double center_x, double center_y, double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_radius = radius;
    this->_error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

circle::circle(const circle &other)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->_error_code = ER_SUCCESS;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return ;
}

circle &circle::operator=(const circle &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = circle::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return (*this);
}

circle::circle(circle &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->_error_code = ER_SUCCESS;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

circle &circle::operator=(circle &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = circle::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return (*this);
}

circle::~circle()
{
    return ;
}

int circle::set_center(double center_x, double center_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int circle::set_center_x(double center_x)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_x = center_x;
    this->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int circle::set_center_y(double center_y)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_y = center_y;
    this->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int circle::set_radius(double radius)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_radius = radius;
    this->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

double  circle::get_center_x() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<circle *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_center_x;
    const_cast<circle *>(this)->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (value);
}

double  circle::get_center_y() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<circle *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_center_y;
    const_cast<circle *>(this)->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (value);
}

double  circle::get_radius() const
{
    int entry_errno;
    double value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<circle *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_radius;
    const_cast<circle *>(this)->set_error(ER_SUCCESS);
    geometry_circle_restore_errno(guard, entry_errno);
    return (value);
}

int circle::get_error() const
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<circle *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    geometry_circle_restore_errno(guard, entry_errno);
    return (error_code);
}

const char  *circle::get_error_str() const
{
    int entry_errno;
    int error_code;
    const char *error_string;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<circle *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    geometry_circle_restore_errno(guard, entry_errno);
    return (error_string);
}

void    circle::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int circle::lock_pair(const circle &first, const circle &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    pt_mutex &first_mutex = first._mutex;
    pt_mutex &second_mutex = second._mutex;

    return (geometry_lock_tracker_lock_pair(&first, &second,
            first_mutex, second_mutex, first_guard, second_guard));
}
