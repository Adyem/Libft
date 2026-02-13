#include "math_autodiff.hpp"

#include <cmath>
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

thread_local int ft_dual_number::_last_error = FT_ERR_SUCCESS;

static void math_autodiff_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void ft_dual_number::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dual_number lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_dual_number::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_dual_number::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dual_number::set_last_operation_error(int error_code) noexcept
{
    ft_dual_number::_last_error = error_code;
    return (error_code);
}

int ft_dual_number::lock_mutex(void) const noexcept
{
    this->abort_if_not_initialized("ft_dual_number::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_dual_number::unlock_mutex(void) const noexcept
{
    this->abort_if_not_initialized("ft_dual_number::unlock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dual_number::lock_pair(const ft_dual_number &first,
    const ft_dual_number &second,
    const ft_dual_number *&lower,
    const ft_dual_number *&upper)
{
    const ft_dual_number *ordered_first;
    const ft_dual_number *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const ft_dual_number *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
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
        math_autodiff_sleep_backoff();
    }
}

void ft_dual_number::unlock_pair(const ft_dual_number *lower,
    const ft_dual_number *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

ft_dual_number::ft_dual_number() noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_dual_number::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    return ;
}

ft_dual_number::ft_dual_number(double value, double derivative) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_dual_number::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    this->_operation_error = this->initialize(value, derivative);
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialized_state == ft_dual_number::_state_uninitialized)
        this->_initialized_state = ft_dual_number::_state_destroyed;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return ;
}

ft_dual_number::ft_dual_number(const ft_dual_number &other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_dual_number::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    this->_operation_error = this->initialize(other);
    if (this->_operation_error == FT_ERR_SUCCESS)
        this->_operation_error = other._operation_error;
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialized_state == ft_dual_number::_state_uninitialized)
        this->_initialized_state = ft_dual_number::_state_destroyed;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return ;
}

ft_dual_number::ft_dual_number(ft_dual_number &&other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_dual_number::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    this->_operation_error = this->initialize(static_cast<ft_dual_number &&>(other));
    if (this->_operation_error == FT_ERR_SUCCESS)
    {
        this->_operation_error = other._operation_error;
        other._operation_error = FT_ERR_SUCCESS;
    }
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialized_state == ft_dual_number::_state_uninitialized)
        this->_initialized_state = ft_dual_number::_state_destroyed;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return ;
}

int ft_dual_number::initialize() noexcept
{
    if (this->_initialized_state == ft_dual_number::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dual_number::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_value = 0.0;
    this->_derivative = 0.0;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = ft_dual_number::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dual_number::initialize(double value, double derivative) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_value = value;
    this->_derivative = derivative;
    this->_operation_error = FT_ERR_SUCCESS;
    return (FT_ERR_SUCCESS);
}

int ft_dual_number::initialize(const ft_dual_number &other) noexcept
{
    int initialize_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != ft_dual_number::_state_initialized)
    {
        if (other._initialized_state == ft_dual_number::_state_uninitialized)
            other.abort_lifecycle_error("ft_dual_number::initialize(const ft_dual_number &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("ft_dual_number::initialize(const ft_dual_number &) source",
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
        this->destroy();
        return (lock_error);
    }
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_operation_error = other._operation_error;
    unlock_error = other.unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_dual_number::move(ft_dual_number &other) noexcept
{
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != ft_dual_number::_state_initialized)
    {
        if (other._initialized_state == ft_dual_number::_state_uninitialized)
            other.abort_lifecycle_error("ft_dual_number::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("ft_dual_number::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != ft_dual_number::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_operation_error = other._operation_error;
    other._value = 0.0;
    other._derivative = 0.0;
    other._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int ft_dual_number::initialize(ft_dual_number &&other) noexcept
{
    int initialize_error;
    int move_error;

    if (other._initialized_state != ft_dual_number::_state_initialized)
    {
        if (other._initialized_state == ft_dual_number::_state_uninitialized)
            other.abort_lifecycle_error("ft_dual_number::initialize(ft_dual_number &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("ft_dual_number::initialize(ft_dual_number &&) source",
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
        this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_dual_number::destroy() noexcept
{
    if (this->_initialized_state != ft_dual_number::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dual_number::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->disable_thread_safety();
    this->_value = 0.0;
    this->_derivative = 0.0;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = ft_dual_number::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

ft_dual_number::~ft_dual_number() noexcept
{
    if (this->_initialized_state == ft_dual_number::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dual_number::~ft_dual_number",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_dual_number::_state_initialized)
        this->destroy();
    return ;
}

ft_dual_number &ft_dual_number::operator=(const ft_dual_number &other) noexcept
{
    int assignment_error;

    if (this == &other)
    {
        this->_operation_error = FT_ERR_SUCCESS;
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (*this);
    }
    if (other._initialized_state != ft_dual_number::_state_initialized)
    {
        if (other._initialized_state == ft_dual_number::_state_uninitialized)
            other.abort_lifecycle_error("ft_dual_number::operator=(const ft_dual_number &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("ft_dual_number::operator=(const ft_dual_number &) source",
                "called with destroyed source object");
        this->_operation_error = FT_ERR_INVALID_STATE;
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (*this);
    }
    if (this->_initialized_state == ft_dual_number::_state_initialized)
    {
        assignment_error = this->destroy();
        if (assignment_error != FT_ERR_SUCCESS)
        {
            this->_operation_error = assignment_error;
            ft_dual_number::set_last_operation_error(this->_operation_error);
            return (*this);
        }
    }
    assignment_error = this->initialize(other);
    if (assignment_error == FT_ERR_SUCCESS)
        this->_operation_error = other._operation_error;
    else
        this->_operation_error = assignment_error;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return (*this);
}

ft_dual_number &ft_dual_number::operator=(ft_dual_number &&other) noexcept
{
    int assignment_error;

    if (other._initialized_state != ft_dual_number::_state_initialized)
    {
        if (other._initialized_state == ft_dual_number::_state_uninitialized)
            other.abort_lifecycle_error("ft_dual_number::operator=(ft_dual_number &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("ft_dual_number::operator=(ft_dual_number &&) source",
                "called with destroyed source object");
        this->_operation_error = FT_ERR_INVALID_STATE;
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (*this);
    }
    if (this == &other)
    {
        this->_operation_error = FT_ERR_SUCCESS;
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (*this);
    }
    if (this->_initialized_state == ft_dual_number::_state_initialized)
    {
        assignment_error = this->destroy();
        if (assignment_error != FT_ERR_SUCCESS)
        {
            this->_operation_error = assignment_error;
            ft_dual_number::set_last_operation_error(this->_operation_error);
            return (*this);
        }
    }
    assignment_error = this->initialize(static_cast<ft_dual_number &&>(other));
    if (assignment_error == FT_ERR_SUCCESS)
    {
        this->_operation_error = other._operation_error;
        other._operation_error = FT_ERR_SUCCESS;
    }
    else
        this->_operation_error = assignment_error;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return (*this);
}

ft_dual_number ft_dual_number::constant(double value) noexcept
{
    return (ft_dual_number(value, 0.0));
}

ft_dual_number ft_dual_number::variable(double value) noexcept
{
    return (ft_dual_number(value, 1.0));
}

int ft_dual_number::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int enable_error;
    int mutex_error;

    this->abort_if_not_initialized("ft_dual_number::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        enable_error = FT_ERR_SUCCESS;
    else
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            enable_error = FT_ERR_NO_MEMORY;
        else
        {
            mutex_error = mutex_pointer->initialize();
            if (mutex_error != FT_ERR_SUCCESS)
            {
                delete mutex_pointer;
                enable_error = mutex_error;
            }
            else
            {
                this->_mutex = mutex_pointer;
                enable_error = FT_ERR_SUCCESS;
            }
        }
    }
    this->_operation_error = enable_error;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return (enable_error);
}

void ft_dual_number::disable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_dual_number::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_last_operation_error(this->_operation_error);
    return ;
}

bool ft_dual_number::is_thread_safe_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_dual_number::is_thread_safe_enabled");
    ft_dual_number::set_last_operation_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

double ft_dual_number::value() const noexcept
{
    int lock_error;
    int unlock_error;
    double result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(lock_error);
        return (0.0);
    }
    result = this->_value;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(unlock_error);
        return (result);
    }
    ft_dual_number::set_last_operation_error(FT_ERR_SUCCESS);
    return (result);
}

double ft_dual_number::derivative() const noexcept
{
    int lock_error;
    int unlock_error;
    double result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(lock_error);
        return (0.0);
    }
    result = this->_derivative;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(unlock_error);
        return (result);
    }
    ft_dual_number::set_last_operation_error(FT_ERR_SUCCESS);
    return (result);
}

ft_dual_number_proxy::ft_dual_number_proxy() noexcept
    : _value(0.0, 0.0)
    , _last_error(FT_ERR_SUCCESS)
{
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(int error_code) noexcept
    : _value(0.0, 0.0)
    , _last_error(error_code)
{
    this->_value._operation_error = error_code;
    ft_dual_number::set_last_operation_error(error_code);
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(const ft_dual_number &value,
    int error_code) noexcept
    : _value(value)
    , _last_error(error_code)
{
    this->_value._operation_error = error_code;
    ft_dual_number::set_last_operation_error(error_code);
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(
    const ft_dual_number_proxy &other) noexcept
    : _value(other._value)
    , _last_error(other._last_error)
{
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(
    ft_dual_number_proxy &&other) noexcept
    : _value(static_cast<ft_dual_number &&>(other._value))
    , _last_error(other._last_error)
{
    other._last_error = FT_ERR_SUCCESS;
    return ;
}

ft_dual_number_proxy::~ft_dual_number_proxy()
{
    return ;
}

ft_dual_number_proxy &ft_dual_number_proxy::operator=(
    const ft_dual_number_proxy &other) noexcept
{
    this->_value = other._value;
    this->_last_error = other._last_error;
    this->_value._operation_error = this->_last_error;
    return (*this);
}

ft_dual_number_proxy &ft_dual_number_proxy::operator=(
    ft_dual_number_proxy &&other) noexcept
{
    this->_value = static_cast<ft_dual_number &&>(other._value);
    this->_last_error = other._last_error;
    this->_value._operation_error = this->_last_error;
    other._last_error = FT_ERR_SUCCESS;
    return (*this);
}

ft_dual_number_proxy ft_dual_number_proxy::operator+(
    const ft_dual_number &other) const noexcept
{
    return (static_cast<ft_dual_number>(*this) + other);
}

ft_dual_number_proxy ft_dual_number_proxy::operator-(
    const ft_dual_number &other) const noexcept
{
    return (static_cast<ft_dual_number>(*this) - other);
}

ft_dual_number_proxy ft_dual_number_proxy::operator*(
    const ft_dual_number &other) const noexcept
{
    return (static_cast<ft_dual_number>(*this) * other);
}

ft_dual_number_proxy ft_dual_number_proxy::operator/(
    const ft_dual_number &other) const noexcept
{
    return (static_cast<ft_dual_number>(*this) / other);
}

ft_dual_number_proxy::operator ft_dual_number() const noexcept
{
    ft_dual_number result(this->_value);

    result._operation_error = this->_last_error;
    ft_dual_number::set_last_operation_error(this->_last_error);
    return (result);
}

int ft_dual_number_proxy::get_error() const noexcept
{
    return (this->_last_error);
}

ft_dual_number_proxy ft_dual_number::operator+(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int initialize_error;
    int lock_error;

    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value + other._value;
    result._derivative = this->_derivative + other._derivative;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator-(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int initialize_error;
    int lock_error;

    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value - other._value;
    result._derivative = this->_derivative - other._derivative;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator*(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int initialize_error;
    int lock_error;

    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value * other._value;
    result._derivative = this->_value * other._derivative
        + this->_derivative * other._value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator/(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int initialize_error;
    int lock_error;
    double denominator;
    double epsilon;

    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_last_operation_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    denominator = other._value;
    epsilon = 0.000000000001;
    if (std::fabs(denominator) <= epsilon)
    {
        result._operation_error = FT_ERR_DIVIDE_BY_ZERO;
        ft_dual_number::unlock_pair(lower, upper);
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value / denominator;
    result._derivative = (this->_derivative * denominator - this->_value * other._derivative)
        / (denominator * denominator);
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number ft_dual_number::apply_sin() const noexcept
{
    ft_dual_number result;
    int initialize_error;
    int lock_error;
    int unlock_error;
    double value;
    double derivative_value;

    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = unlock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    result._value = std::sin(value);
    result._derivative = std::cos(value) * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_cos() const noexcept
{
    ft_dual_number result;
    int initialize_error;
    int lock_error;
    int unlock_error;
    double value;
    double derivative_value;

    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = unlock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    result._value = std::cos(value);
    result._derivative = -std::sin(value) * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_exp() const noexcept
{
    ft_dual_number result;
    int initialize_error;
    int lock_error;
    int unlock_error;
    double value;
    double derivative_value;
    double exponential;

    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = unlock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    exponential = std::exp(value);
    result._value = exponential;
    result._derivative = exponential * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_log() const noexcept
{
    ft_dual_number result;
    int initialize_error;
    int lock_error;
    int unlock_error;
    double value;
    double derivative_value;

    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = unlock_error;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    if (value <= 0.0)
    {
        result._operation_error = FT_ERR_INVALID_ARGUMENT;
        ft_dual_number::set_last_operation_error(result._operation_error);
        return (result);
    }
    result._value = std::log(value);
    result._derivative = derivative_value / value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_last_operation_error(result._operation_error);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_dual_number::get_mutex_for_validation() const noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_initialized_state != ft_dual_number::_state_initialized)
        return (ft_nullptr);
    if (this->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            return (ft_nullptr);
        mutex_error = mutex_pointer->initialize();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (ft_nullptr);
        }
        const_cast<ft_dual_number *>(this)->_mutex = mutex_pointer;
    }
    return (this->_mutex);
}
#endif

int ft_dual_number::last_operation_error() noexcept
{
    return (ft_dual_number::_last_error);
}

const char *ft_dual_number::last_operation_error_str() noexcept
{
    return (ft_strerror(ft_dual_number::last_operation_error()));
}

static int math_autodiff_prepare_inputs(const ft_vector<double> &point,
    size_t active_index, ft_vector<ft_dual_number> &dual_inputs) noexcept
{
    size_t dimension;
    size_t index;

    dual_inputs.clear();
    dimension = point.size();
    dual_inputs.reserve(dimension);
    index = 0;
    while (index < dimension)
    {
        ft_dual_number variable;
        double value;

        value = point[index];
        if (index == active_index)
            variable = ft_dual_number::variable(value);
        else
            variable = ft_dual_number::constant(value);
        if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
            return (-1);
        dual_inputs.push_back(variable);
        index++;
    }
    return (0);
}

int math_autodiff_univariate(math_autodiff_univariate_function function,
    double point, double *value, double *derivative, void *user_data) noexcept
{
    ft_dual_number variable;
    ft_dual_number result;

    if (function == ft_nullptr || value == ft_nullptr || derivative == ft_nullptr)
        return (-1);
    variable = ft_dual_number::variable(point);
    if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    result = function(variable, user_data);
    if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    *value = result.value();
    if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    *derivative = result.derivative();
    if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int math_autodiff_gradient(math_autodiff_multivariate_function function,
    const ft_vector<double> &point, ft_vector<double> &gradient,
    double *value, void *user_data) noexcept
{
    size_t dimension;
    size_t index;
    bool value_set;

    if (function == ft_nullptr)
        return (-1);
    dimension = point.size();
    gradient.clear();
    gradient.reserve(dimension);
    index = 0;
    value_set = false;
    while (index < dimension)
    {
        ft_vector<ft_dual_number> dual_inputs;
        ft_dual_number result;

        if (math_autodiff_prepare_inputs(point, index, dual_inputs) != 0)
        {
            gradient.clear();
            return (-1);
        }
        result = function(dual_inputs, user_data);
        if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        {
            gradient.clear();
            return (-1);
        }
        if (!value_set && value != ft_nullptr)
        {
            *value = result.value();
            if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
            {
                gradient.clear();
                return (-1);
            }
            value_set = true;
        }
        gradient.push_back(result.derivative());
        if (ft_dual_number::last_operation_error() != FT_ERR_SUCCESS)
        {
            gradient.clear();
            return (-1);
        }
        index++;
    }
    return (0);
}
