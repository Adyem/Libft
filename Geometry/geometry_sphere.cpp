#include "geometry_sphere.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "geometry_lock_tracker.hpp"


static int sphere_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int sphere_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

void sphere::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

sphere::sphere()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_center_z = 0.0;
    this->_radius = 0.0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

sphere::sphere(double center_x, double center_y, double center_z,
        double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_center_z = center_z;
    this->_radius = radius;
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_center_z = other._center_z;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
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
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
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
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

sphere::~sphere()
{
    return ;
}

int sphere::set_center(double center_x, double center_y, double center_z)
{
    int lock_error;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_center_z = center_z;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_x(double center_x)
{
    int lock_error;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_y(double center_y)
{
    int lock_error;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_y = center_y;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_center_z(double center_z)
{
    int lock_error;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_z = center_z;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int sphere::set_radius(double radius)
{
    int lock_error;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_radius = radius;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  sphere::get_center_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_center_x;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_center_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_center_y;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_center_z() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_center_z;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  sphere::get_radius() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = sphere_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_radius;
    unlock_error = sphere_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        int self_error = sphere_lock_mutex(this->_mutex);
        if (self_error != FT_ERR_SUCCESSS)
            this->record_operation_error(self_error);
        return (self_error);
    }
    if (ordered_first > ordered_second)
    {
        ordered_first = &other;
        ordered_second = this;
    }
    lower = ordered_first;
    upper = ordered_second;
    int lock_error;

    lock_error = geometry_lock_tracker_lock_pair(lower, upper,
            lower->_mutex, upper->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    return (FT_ERR_SUCCESSS);
}

void sphere::unlock_pair(const sphere *lower, const sphere *upper)
{
    if (upper != ft_nullptr)
        sphere_unlock_mutex(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        sphere_unlock_mutex(lower->_mutex);
    return ;
}

pt_recursive_mutex *sphere::get_mutex_for_validation() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *sphere::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *sphere::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
