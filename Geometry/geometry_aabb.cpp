#include "geometry_aabb.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"

#include <cstddef>
#include <utility>
#include "../Template/move.hpp"

static void aabb_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int aabb::lock_mutex() const noexcept
{
    return (pt_recursive_mutex_lock_if_valid(this->_mutex));
}

int aabb::unlock_mutex() const noexcept
{
    return (pt_recursive_mutex_unlock_if_valid(this->_mutex));
}

int aabb::lock_pair(const aabb &other, const aabb *&lower, const aabb *&upper) const
{
    const aabb *ordered_first = this;
    const aabb *ordered_second = &other;

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
        aabb_sleep_backoff();
    }
}

void aabb::unlock_pair(const aabb *lower, const aabb *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

aabb::aabb()
{
    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(lock_error);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(lock_error);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

aabb::~aabb()
{
    this->disable_thread_safety();
    return ;
}

int aabb::set_bounds(double minimum_x, double minimum_y,
        double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum(double minimum_x, double minimum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (minimum_x > this->_maximum_x || minimum_y > this->_maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum_x(double minimum_x)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (minimum_x > this->_maximum_x)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_minimum_y(double minimum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (minimum_y > this->_maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_y = minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum(double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (maximum_x < this->_minimum_x || maximum_y < this->_minimum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum_x(double maximum_x)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (maximum_x < this->_minimum_x)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int aabb::set_maximum_y(double maximum_y)
{
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    if (maximum_y < this->_minimum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (unlock_error);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

double  aabb::get_minimum_x() const
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
    value = this->_minimum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_minimum_y() const
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
    value = this->_minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_maximum_x() const
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
    value = this->_maximum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  aabb::get_maximum_y() const
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
    value = this->_maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

int aabb::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void aabb::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool aabb::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *aabb::get_mutex_for_testing() noexcept
{
    if (this->_mutex == ft_nullptr)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

int aabb::prepare_thread_safety(void) noexcept
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

void aabb::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
