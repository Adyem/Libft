#include "geometry_circle.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static void circle_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void circle::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "circle lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void circle::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == circle::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int circle::lock_pair(const circle &other, const circle *&lower,
    const circle *&upper) const
{
    const circle *ordered_first;
    const circle *ordered_second;

    ordered_first = this;
    ordered_second = &other;
    if (ordered_first == ordered_second)
    {
        lower = this;
        upper = this;
        return (pt_recursive_mutex_lock_if_not_null(this->_mutex));
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

        lower_error = pt_recursive_mutex_lock_if_not_null(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        upper_error = pt_recursive_mutex_lock_if_not_null(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
            return (upper_error);
        }
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
        circle_sleep_backoff();
    }
}

void circle::unlock_pair(const circle *lower, const circle *upper)
{
    if (upper != ft_nullptr)
        pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

circle::circle()
    : _center_x(0.0)
    , _center_y(0.0)
    , _radius(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(circle::_state_uninitialized)
{
    return ;
}

circle::circle(double center_x, double center_y, double radius)
    : _center_x(0.0)
    , _center_y(0.0)
    , _radius(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(circle::_state_uninitialized)
{
    int initialize_error;

    initialize_error = this->initialize(center_x, center_y, radius);
    if (initialize_error != FT_ERR_SUCCESS
        && this->_initialized_state == circle::_state_uninitialized)
        this->_initialized_state = circle::_state_destroyed;
    return ;
}

int circle::initialize() noexcept
{
    if (this->_initialized_state == circle::_state_initialized)
    {
        this->abort_lifecycle_error("circle::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->_initialized_state = circle::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int circle::initialize(double center_x, double center_y, double radius) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_center_x = center_x;
    this->_center_y = center_y;
    this->_radius = radius;
    return (FT_ERR_SUCCESS);
}

int circle::initialize(const circle &other) noexcept
{
    int initialize_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != circle::_state_initialized)
    {
        if (other._initialized_state == circle::_state_uninitialized)
            other.abort_lifecycle_error("circle::initialize(const circle &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("circle::initialize(const circle &) source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int circle::move(circle &other) noexcept
{
    const circle *lower;
    const circle *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != circle::_state_initialized)
    {
        if (other._initialized_state == circle::_state_uninitialized)
            other.abort_lifecycle_error("circle::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("circle::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != circle::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = this->lock_pair(other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_center_x = other._center_x;
    this->_center_y = other._center_y;
    this->_radius = other._radius;
    other._center_x = 0.0;
    other._center_y = 0.0;
    other._radius = 0.0;
    this->unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int circle::initialize(circle &&other) noexcept
{
    int initialize_error;
    int move_error;

    if (other._initialized_state != circle::_state_initialized)
    {
        if (other._initialized_state == circle::_state_uninitialized)
            other.abort_lifecycle_error("circle::initialize(circle &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("circle::initialize(circle &&) source",
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

int circle::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != circle::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_center_x = 0.0;
    this->_center_y = 0.0;
    this->_radius = 0.0;
    this->_initialized_state = circle::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

circle::~circle()
{
    if (this->_initialized_state == circle::_state_initialized)
        (void)this->destroy();
    return ;
}

int circle::set_center(double center_x, double center_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("circle::set_center");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_center_x = center_x;
    this->_center_y = center_y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int circle::set_center_x(double center_x)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("circle::set_center_x");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_center_x = center_x;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int circle::set_center_y(double center_y)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("circle::set_center_y");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_center_y = center_y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int circle::set_radius(double radius)
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("circle::set_radius");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_radius = radius;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

double circle::get_center_x() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("circle::get_center_x");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_center_x;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

double circle::get_center_y() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("circle::get_center_y");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_center_y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

double circle::get_radius() const
{
    int lock_error;
    int unlock_error;
    double value;

    this->abort_if_not_initialized("circle::get_radius");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_radius;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

int circle::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("circle::enable_thread_safety");
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

int circle::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("circle::disable_thread_safety");
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

bool circle::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *circle::get_mutex_for_testing() noexcept
{
    if (this->_initialized_state != circle::_state_initialized)
        return (ft_nullptr);
    if (this->_mutex == ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (ft_nullptr);
    }
    return (this->_mutex);
}
#endif
