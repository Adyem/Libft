#include "geometry_aabb.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
#include "../Template/move.hpp"

static void aabb_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static int aabb_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int aabb_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

void aabb::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

int aabb::lock_pair(const aabb &other, const aabb *&lower, const aabb *&upper) const
{
    const aabb *ordered_first = this;
    const aabb *ordered_second = &other;

    if (ordered_first == ordered_second)
    {
        lower = this;
        upper = this;
        int self_error = aabb_lock_mutex(this->_mutex);
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
    while (true)
    {
        int lower_error = aabb_lock_mutex(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(lower_error);
            return (lower_error);
        }
        int upper_error = aabb_lock_mutex(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            aabb_unlock_mutex(lower->_mutex);
            this->record_operation_error(upper_error);
            return (upper_error);
        }
        aabb_unlock_mutex(lower->_mutex);
        aabb_sleep_backoff();
    }
}

void aabb::unlock_pair(const aabb *lower, const aabb *upper)
{
    if (upper != ft_nullptr)
        aabb_unlock_mutex(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        aabb_unlock_mutex(lower->_mutex);
    return ;
}

aabb::aabb()
{
    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

aabb::aabb(double minimum_x, double minimum_y,
        double maximum_x, double maximum_y)
{
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

aabb::aabb(const aabb &other)
    : _minimum_x(0.0), _minimum_y(0.0),
    _maximum_x(0.0), _maximum_y(0.0)
{
    const aabb *lower;
    const aabb *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

aabb &aabb::operator=(const aabb &other)
{
    const aabb *lower;
    const aabb *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

aabb::aabb(aabb &&other) noexcept
    : _minimum_x(0.0), _minimum_y(0.0),
    _maximum_x(0.0), _maximum_y(0.0)
{
    const aabb *lower;
    const aabb *upper;
    int lock_error;

    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    other._minimum_x = 0.0;
    other._minimum_y = 0.0;
    other._maximum_x = 0.0;
    other._maximum_y = 0.0;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

aabb &aabb::operator=(aabb &&other) noexcept
{
    const aabb *lower;
    const aabb *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    other._minimum_x = 0.0;
    other._minimum_y = 0.0;
    other._maximum_x = 0.0;
    other._maximum_y = 0.0;
    this->unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

aabb::~aabb()
{
    return ;
}

int aabb::set_bounds(double minimum_x, double minimum_y,
        double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum(double minimum_x, double minimum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (minimum_x > this->_maximum_x || minimum_y > this->_maximum_y)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum_x(double minimum_x)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (minimum_x > this->_maximum_x)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum_y(double minimum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (minimum_y > this->_maximum_y)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_y = minimum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum(double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (maximum_x < this->_minimum_x || maximum_y < this->_minimum_y)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum_x(double maximum_x)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (maximum_x < this->_minimum_x)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum_y(double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (maximum_y < this->_minimum_y)
    {
        unlock_error = aabb_unlock_mutex(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(unlock_error);
            return (unlock_error);
        }
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_y = maximum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (unlock_error);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  aabb::get_minimum_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_minimum_x;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_minimum_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_minimum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_maximum_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_maximum_x;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_maximum_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = aabb_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_maximum_y;
    unlock_error = aabb_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

pt_recursive_mutex *aabb::get_mutex_for_validation() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *aabb::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *aabb::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
