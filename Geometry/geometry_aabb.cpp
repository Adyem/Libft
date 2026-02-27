#include "geometry_aabb.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static void aabb_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void aabb::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "aabb lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void aabb::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == aabb::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int aabb::lock_mutex() const noexcept
{
    this->abort_if_not_initialized("aabb::lock_mutex");
    return (pt_recursive_mutex_lock_if_not_null(this->_mutex));
}

int aabb::unlock_mutex() const noexcept
{
    this->abort_if_not_initialized("aabb::unlock_mutex");
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int aabb::lock_pair(const aabb &other, const aabb *&lower,
    const aabb *&upper) const
{
    const aabb *ordered_first;
    const aabb *ordered_second;

    ordered_first = this;
    ordered_second = &other;
    if (ordered_first == ordered_second)
    {
        lower = this;
        upper = this;
        return (this->lock_mutex());
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
        int lower_error;
        int upper_error;

        lower_error = lower->lock_mutex();
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        upper_error = upper->lock_mutex();
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
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
    : _minimum_x(0.0)
    , _minimum_y(0.0)
    , _maximum_x(0.0)
    , _maximum_y(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(aabb::_state_uninitialized)
{
    return ;
}

aabb::aabb(double minimum_x, double minimum_y,
    double maximum_x, double maximum_y)
    : _minimum_x(0.0)
    , _minimum_y(0.0)
    , _maximum_x(0.0)
    , _maximum_y(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(aabb::_state_uninitialized)
{
    int initialize_error;

    initialize_error = this->initialize(minimum_x, minimum_y,
            maximum_x, maximum_y);
    if (initialize_error != FT_ERR_SUCCESS
        && this->_initialized_state == aabb::_state_uninitialized)
        this->_initialized_state = aabb::_state_destroyed;
    return ;
}

int aabb::initialize() noexcept
{
    if (this->_initialized_state == aabb::_state_initialized)
    {
        this->abort_lifecycle_error("aabb::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->_initialized_state = aabb::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int aabb::initialize(double minimum_x, double minimum_y,
    double maximum_x, double maximum_y) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        (void)this->destroy();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    return (FT_ERR_SUCCESS);
}

int aabb::initialize(const aabb &other) noexcept
{
    int initialize_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != aabb::_state_initialized)
    {
        if (other._initialized_state == aabb::_state_uninitialized)
            other.abort_lifecycle_error("aabb::initialize(const aabb &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("aabb::initialize(const aabb &) source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    unlock_error = other.unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int aabb::move(aabb &other) noexcept
{
    const aabb *lower;
    const aabb *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != aabb::_state_initialized)
    {
        if (other._initialized_state == aabb::_state_uninitialized)
            other.abort_lifecycle_error("aabb::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("aabb::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != aabb::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_minimum_x = other._minimum_x;
    this->_minimum_y = other._minimum_y;
    this->_maximum_x = other._maximum_x;
    this->_maximum_y = other._maximum_y;
    other._minimum_x = 0.0;
    other._minimum_y = 0.0;
    other._maximum_x = 0.0;
    other._maximum_y = 0.0;
    this->unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int aabb::initialize(aabb &&other) noexcept
{
    int initialize_error;
    int move_error;

    if (other._initialized_state != aabb::_state_initialized)
    {
        if (other._initialized_state == aabb::_state_uninitialized)
            other.abort_lifecycle_error("aabb::initialize(aabb &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("aabb::initialize(aabb &&) source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int aabb::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != aabb::_state_initialized)
    {
        this->abort_lifecycle_error("aabb::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_minimum_x = 0.0;
    this->_minimum_y = 0.0;
    this->_maximum_x = 0.0;
    this->_maximum_y = 0.0;
    this->_initialized_state = aabb::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

aabb::~aabb()
{
    if (this->_initialized_state == aabb::_state_initialized)
        (void)this->destroy();
    return ;
}

int aabb::set_bounds(double minimum_x, double minimum_y,
    double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_bounds");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (minimum_x > maximum_x || minimum_y > maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_minimum(double minimum_x, double minimum_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_minimum");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (minimum_x > this->_maximum_x || minimum_y > this->_maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    this->_minimum_y = minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_minimum_x(double minimum_x)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_minimum_x");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (minimum_x > this->_maximum_x)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_x = minimum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_minimum_y(double minimum_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_minimum_y");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (minimum_y > this->_maximum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_minimum_y = minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_maximum(double maximum_x, double maximum_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_maximum");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (maximum_x < this->_minimum_x || maximum_y < this->_minimum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_maximum_x(double maximum_x)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_maximum_x");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (maximum_x < this->_minimum_x)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_x = maximum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int aabb::set_maximum_y(double maximum_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("aabb::set_maximum_y");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (maximum_y < this->_minimum_y)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_maximum_y = maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

double aabb::get_minimum_x() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("aabb::get_minimum_x");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_minimum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

double aabb::get_minimum_y() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("aabb::get_minimum_y");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_minimum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

double aabb::get_maximum_x() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("aabb::get_maximum_x");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_maximum_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

double aabb::get_maximum_y() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("aabb::get_maximum_y");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_maximum_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

int aabb::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("aabb::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int aabb::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("aabb::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (FT_ERR_SUCCESS);
}

bool aabb::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *aabb::get_mutex_for_testing() noexcept
{
    if (this->_initialized_state != aabb::_state_initialized)
        return (ft_nullptr);
    if (this->_mutex == ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (ft_nullptr);
    }
    return (this->_mutex);
}
#endif
