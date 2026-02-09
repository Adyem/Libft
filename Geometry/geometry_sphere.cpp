#include "geometry_sphere.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"

#include <cstddef>

static void sphere_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int sphere::lock_mutex() const noexcept
{
    return (pt_recursive_mutex_lock_if_valid(this->_mutex));
}

int sphere::unlock_mutex() const noexcept
{
    return (pt_recursive_mutex_unlock_if_valid(this->_mutex));
}

sphere::sphere()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_center_z = 0.0;
    this->_radius = 0.0;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

sphere::sphere(double center_x, double center_y, double center_z,
        double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_center_z = center_z;
    this->_radius = radius;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

sphere::sphere(const sphere &other)
    : _center_x(0.0), _center_y(0.0), _center_z(0.0), _radius(0.0)
{
    const sphere *lower;
    const sphere *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

sphere &sphere::operator=(const sphere &other)
{
    const sphere *lower;
    const sphere *upper;
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
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

sphere::sphere(sphere &&other) noexcept
    : _center_x(0.0), _center_y(0.0), _center_z(0.0), _radius(0.0)
{
    const sphere *lower;
    const sphere *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._center_z = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

sphere &sphere::operator=(sphere &&other) noexcept
{
    const sphere *lower;
    const sphere *upper;
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
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._center_z = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

sphere::~sphere()
{
    this->disable_thread_safety();
    return ;
}

int sphere::set_center(double center_x, double center_y, double center_z)
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
    this->_center_z = center_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_x(double center_x)
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

int sphere::set_center_y(double center_y)
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

int sphere::set_center_z(double center_z)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    this->_center_z = center_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_radius(double radius)
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

double  sphere::get_center_x() const
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

double  sphere::get_center_y() const
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

double  sphere::get_center_z() const
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
    value = this->_center_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_radius() const
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

int sphere::lock_pair(const sphere &other, const sphere *&lower,
        const sphere *&upper) const
{
    const sphere *ordered_first = this;
    const sphere *ordered_second = &other;

    if (ordered_first == ordered_second)
    {
        lower = this;
        upper = this;
        int self_error = this->lock_mutex();
        if (self_error != FT_ERR_SUCCESSS)
            ft_global_error_stack_push(self_error);
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
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            ft_global_error_stack_push(upper_error);
            return (upper_error);
        }
        lower->unlock_mutex();
        sphere_sleep_backoff();
    }
}

void sphere::unlock_pair(const sphere *lower, const sphere *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

int sphere::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void sphere::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool sphere::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *sphere::get_mutex_for_testing() noexcept
{
    if (this->_mutex == ft_nullptr)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

int sphere::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void sphere::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
