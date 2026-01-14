#include "geometry_sphere.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "geometry_lock_tracker.hpp"

#include <cstddef>
#include <utility>
#include "../Template/move.hpp"

sphere::sphere()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_center_z = 0.0;
    this->_radius = 0.0;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

sphere::sphere(double center_x, double center_y, double center_z, double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_center_z = center_z;
    this->_radius = radius;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

sphere::sphere(const sphere &other)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_center_z = 0.0;
    this->_radius = 0.0;
    this->_error_code = FT_ERR_SUCCESSS;
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

sphere &sphere::operator=(const sphere &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = sphere::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

sphere::sphere(sphere &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_center_z = 0.0;
    this->_radius = 0.0;
    this->_error_code = FT_ERR_SUCCESSS;
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._center_z = 0.0;
    other._radius = 0.0;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

sphere &sphere::operator=(sphere &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = sphere::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->_error_code = other._error_code;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._center_z = 0.0;
    other._radius = 0.0;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

sphere::~sphere()
{
    return ;
}

int sphere::set_center(double center_x, double center_y, double center_z)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_center_z = center_z;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_x(double center_x)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_x = center_x;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_y(double center_y)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_y = center_y;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_z(double center_z)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_center_z = center_z;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_radius(double radius)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    this->_radius = radius;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  sphere::get_center_x() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    double value;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_center_x;
    const_cast<sphere *>(this)->set_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_center_y() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    double value;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_center_y;
    const_cast<sphere *>(this)->set_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_center_z() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    double value;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_center_z;
    const_cast<sphere *>(this)->set_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_radius() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    double value;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (0.0);
    }
    value = this->_radius;
    const_cast<sphere *>(this)->set_error(FT_ERR_SUCCESSS);
    return (value);
}

int sphere::get_error() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    int error_code;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    return (error_code);
}

const char  *sphere::get_error_str() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    int error_code;
    const char *error_string;

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<sphere *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    return (error_string);
}

void    sphere::set_error(int error_code) const
{
    this->_error_code = error_code;
    return ;
}

int sphere::lock_pair(const sphere &first, const sphere &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    pt_mutex &first_mutex = first._mutex;
    pt_mutex &second_mutex = second._mutex;

    return (geometry_lock_tracker_lock_pair(&first, &second,
            first_mutex, second_mutex, first_guard, second_guard));
}
