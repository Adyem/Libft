#include "geometry_aabb.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"

#include <chrono>
#include <cstddef>
#include <random>
#include <utility>
#include "../Template/move.hpp"

static void geometry_aabb_sleep_backoff()
{
    static thread_local bool generator_initialized = false;
    static thread_local std::minstd_rand generator;
    std::uniform_int_distribution<int> distribution(1, 10);
    unsigned long long time_seed;
    std::size_t address_seed;
    unsigned int combined_seed;
    int delay_ms;

    if (!generator_initialized)
    {
        time_seed = static_cast<unsigned long long>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        address_seed = reinterpret_cast<std::size_t>(&generator);
        combined_seed = static_cast<unsigned int>(time_seed ^ address_seed);
        if (combined_seed == 0)
            combined_seed = static_cast<unsigned int>(address_seed | 1U);
        generator.seed(combined_seed);
        generator_initialized = true;
    }
    delay_ms = distribution(generator);
    pt_thread_sleep(static_cast<unsigned int>(delay_ms));
    return ;
}

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
    const aabb *ordered_first;
    const aabb *ordered_second;
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
        const aabb *temporary;

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
        geometry_aabb_sleep_backoff();
    }
}
