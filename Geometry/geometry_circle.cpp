#include "geometry_circle.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"

#include <cstddef>

static void circle_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int circle::lock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int circle::unlock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int circle::lock_pair(const circle &other, const circle *&lower,
        const circle *&upper) const
{
    const circle *ordered_first = this;
    const circle *ordered_second = &other;

    if (ordered_first == ordered_second)
    {
        lower = this;
        upper = this;
        int self_error = this->lock_mutex();
        if (self_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(self_error);
        }
        return (self_error);
    }
    if (ordered_first > ordered_second)
    {
        ordered_first = &other;
        ordered_second = this;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = lower->lock_mutex();
        if (lower_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lower_error);
            return (lower_error);
        }
        int upper_error = upper->lock_mutex();
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            ft_global_error_stack_push(upper_error);
            return (upper_error);
        }
        lower->unlock_mutex();
        circle_sleep_backoff();
    }
}

void circle::unlock_pair(const circle *lower, const circle *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

circle::circle()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

circle::circle(double center_x, double center_y, double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_radius = radius;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

circle::circle(const circle &other)
    : _center_x(0.0), _center_y(0.0), _radius(0.0)
{
    const circle *lower;
    const circle *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

circle &circle::operator=(const circle &other)
{
    const circle *lower;
    const circle *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

circle::circle(circle &&other) noexcept
    : _center_x(0.0), _center_y(0.0), _radius(0.0)
{
    const circle *lower;
    const circle *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

circle &circle::operator=(circle &&other) noexcept
{
    const circle *lower;
    const circle *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

circle::~circle()
{
    return ;
}

int circle::set_center(double center_x, double center_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    this->_center_y = center_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_center_x(double center_x)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_center_y(double center_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    this->_center_y = center_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_radius(double radius)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    this->_radius = radius;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  circle::get_center_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_center_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  circle::get_center_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_center_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  circle::get_radius() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_radius;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

bool    intersect_circle(const circle &first, const circle &second)
{
    bool result;

    circle &first_ref = const_cast<circle &>(first);
    circle &second_ref = const_cast<circle &>(second);
    const circle *lower;
    const circle *upper;
    int lock_error;
    double  delta_x;
    double  delta_y;
    double  radius_sum;
    double  distance_squared;

    result = false;
    lock_error = first.lock_pair(second, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    first.unlock_pair(lower, upper);
    result = true;
    if (distance_squared > radius_sum * radius_sum)
        result = false;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    if (result)
        return (true);
    return (false);
}

int circle::enable_thread_safety() noexcept
{
    this->_thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void circle::disable_thread_safety() noexcept
{
    this->_thread_safe_enabled = false;
    return ;
}

bool circle::is_thread_safe_enabled() const noexcept
{
    return (this->_thread_safe_enabled);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *circle::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
