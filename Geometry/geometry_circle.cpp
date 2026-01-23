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

static pt_recursive_mutex *circle_mutex_for(const circle &value)
{
    return (value.get_mutex_for_validation());
}

static int circle_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int circle_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

void circle::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
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
        int self_error = circle_lock_mutex(this->_mutex);
        if (self_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(self_error);
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
    pt_recursive_mutex *lower_mutex = circle_mutex_for(*lower);
    pt_recursive_mutex *upper_mutex = circle_mutex_for(*upper);
    while (true)
    {
        int lower_error = circle_lock_mutex(*lower_mutex);
        if (lower_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(lower_error);
            return (lower_error);
        }
        int upper_error = circle_lock_mutex(*upper_mutex);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            circle_unlock_mutex(*lower_mutex);
            this->record_operation_error(upper_error);
            return (upper_error);
        }
        circle_unlock_mutex(*lower_mutex);
        circle_sleep_backoff();
    }
}

void circle::unlock_pair(const circle *lower, const circle *upper)
{
    if (upper != ft_nullptr)
        circle_unlock_mutex(*circle_mutex_for(*upper));
    if (lower != ft_nullptr && lower != upper)
        circle_unlock_mutex(*circle_mutex_for(*lower));
    return ;
}

circle::circle()
{
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

circle::circle(double center_x, double center_y, double radius)
{
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_radius = radius;
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return ;
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
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

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    this->_center_y = center_y;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_center_x(double center_x)
{
    int lock_error;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_x = center_x;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_center_y(double center_y)
{
    int lock_error;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_center_y = center_y;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int circle::set_radius(double radius)
{
    int lock_error;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    this->_radius = radius;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  circle::get_center_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_center_x;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  circle::get_center_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_center_y;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  circle::get_radius() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = circle_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_radius;
    unlock_error = circle_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
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
        first_ref.record_operation_error(lock_error);
        second_ref.record_operation_error(lock_error);
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y;
    first_ref.record_operation_error(FT_ERR_SUCCESSS);
    second_ref.record_operation_error(FT_ERR_SUCCESSS);
    first.unlock_pair(lower, upper);
    result = true;
    if (distance_squared > radius_sum * radius_sum)
        result = false;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    if (result)
        return (true);
    return (false);
}

pt_recursive_mutex *circle::get_mutex_for_validation() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *circle::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *circle::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
