#include "ft_dual_number.hpp"

#include <cmath>
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"

thread_local int32_t ft_dual_number::_last_error = FT_ERR_SUCCESS;

static void math_autodiff_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void ft_dual_number::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void ft_dual_number::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t ft_dual_number::set_error(int32_t error_code) noexcept
{
    ft_dual_number::_last_error = error_code;
    return (error_code);
}

int32_t ft_dual_number::lock_pair(const ft_dual_number &first,
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
        return (pt_recursive_mutex_lock_if_not_null(first._mutex));
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
    while (FT_TRUE)
    {
        int32_t lower_error;
        int32_t upper_error;

        lower_error = pt_recursive_mutex_lock_if_not_null(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        upper_error = pt_recursive_mutex_lock_if_not_null(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
            return (upper_error);
        }
        (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
        math_autodiff_sleep_backoff();
    }
}

void ft_dual_number::unlock_pair(const ft_dual_number *lower,
    const ft_dual_number *upper)
{
    if (upper != ft_nullptr)
        (void)pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

ft_dual_number::ft_dual_number() noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _operation_error(FT_ERR_SUCCESS)
{
    return ;
}

ft_dual_number::ft_dual_number(double value, double derivative) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _operation_error(FT_ERR_SUCCESS)
{
    uint32_t previous_last_error;

    previous_last_error = ft_dual_number::_last_error;
    this->_operation_error = this->initialize(value, derivative);
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    ft_dual_number::set_error(previous_last_error);
    return ;
}

ft_dual_number::ft_dual_number(const ft_dual_number &other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _operation_error(FT_ERR_SUCCESS)
{
    uint32_t previous_last_error;

    previous_last_error = ft_dual_number::_last_error;
    this->_operation_error = this->initialize(other);
    if (this->_operation_error == FT_ERR_SUCCESS)
        this->_operation_error = other._operation_error;
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    ft_dual_number::set_error(previous_last_error);
    return ;
}

ft_dual_number::ft_dual_number(ft_dual_number &&other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _operation_error(FT_ERR_SUCCESS)
{
    uint32_t previous_last_error;

    previous_last_error = ft_dual_number::_last_error;
    this->_operation_error = this->initialize(static_cast<ft_dual_number &&>(other));
    if (this->_operation_error == FT_ERR_SUCCESS)
    {
        this->_operation_error = other._operation_error;
        other._operation_error = FT_ERR_SUCCESS;
    }
    if (this->_operation_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    ft_dual_number::set_error(previous_last_error);
    return ;
}

int32_t ft_dual_number::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("ft_dual_number::initialize",
            "called while object is already initialised");
        return (ft_dual_number::set_error(FT_ERR_INVALID_STATE));
    }
    this->_value = 0.0;
    this->_derivative = 0.0;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

int32_t ft_dual_number::initialize(double value, double derivative) noexcept
{
    int32_t initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (ft_dual_number::set_error(static_cast<uint32_t>(initialize_error)));
    this->_value = value;
    this->_derivative = derivative;
    this->_operation_error = FT_ERR_SUCCESS;
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

int32_t ft_dual_number::initialize(const ft_dual_number &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_dual_number::initialize(const ft_dual_number &) source",
            "called with uninitialised source object");
        return (ft_dual_number::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_value = 0.0;
        this->_derivative = 0.0;
        this->_operation_error = FT_ERR_SUCCESS;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (ft_dual_number::set_error(static_cast<uint32_t>(initialize_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->destroy();
        return (ft_dual_number::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_operation_error = other._operation_error;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

int32_t ft_dual_number::move(ft_dual_number &other) noexcept
{
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_dual_number::move source",
            "called with uninitialised source object");
        return (ft_dual_number::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_value = 0.0;
        this->_derivative = 0.0;
        this->_operation_error = FT_ERR_SUCCESS;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (ft_dual_number::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_value = 0.0;
        this->_derivative = 0.0;
        this->_operation_error = FT_ERR_SUCCESS;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_dual_number::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_operation_error = other._operation_error;
    other._value = 0.0;
    other._derivative = 0.0;
    other._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

int32_t ft_dual_number::initialize(ft_dual_number &&other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_dual_number::initialize(ft_dual_number &&) source",
            "called with uninitialised source object");
        return (ft_dual_number::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_value = 0.0;
        this->_derivative = 0.0;
        this->_operation_error = FT_ERR_SUCCESS;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (ft_dual_number::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (ft_dual_number::set_error(static_cast<uint32_t>(initialize_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->destroy();
        return (ft_dual_number::set_error(static_cast<uint32_t>(move_error)));
    }
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

int32_t ft_dual_number::destroy() noexcept
{
    int32_t first_error;
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (ft_dual_number::set_error(FT_ERR_SUCCESS));
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = disable_error;
    this->_value = 0.0;
    this->_derivative = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (first_error != FT_ERR_SUCCESS)
    {
        this->_operation_error = first_error;
        return (ft_dual_number::set_error(static_cast<uint32_t>(first_error)));
    }
    this->_operation_error = FT_ERR_SUCCESS;
    return (ft_dual_number::set_error(FT_ERR_SUCCESS));
}

ft_dual_number::~ft_dual_number() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = ft_dual_number::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        this->destroy();
    ft_dual_number::set_error(previous_last_error);
    return ;
}

ft_dual_number ft_dual_number::constant(double value) noexcept
{
    ft_dual_number result(value, 0.0);

    ft_dual_number::set_error(static_cast<uint32_t>(result._operation_error));
    return (result);
}

ft_dual_number ft_dual_number::variable(double value) noexcept
{
    ft_dual_number result(value, 1.0);

    ft_dual_number::set_error(static_cast<uint32_t>(result._operation_error));
    return (result);
}

int32_t ft_dual_number::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t enable_error;
    int32_t mutex_error;

    this->abort_if_not_initialised("ft_dual_number::enable_thread_safety");
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
    ft_dual_number::set_error(this->_operation_error);
    return (enable_error);
}

int32_t ft_dual_number::disable_thread_safety() noexcept
{
    int32_t mutex_error;

    this->abort_if_not_initialised("ft_dual_number::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
        if (mutex_error != FT_ERR_SUCCESS)
        {
            this->_operation_error = mutex_error;
            ft_dual_number::set_error(this->_operation_error);
            return (mutex_error);
        }
    }
    this->_operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_error(this->_operation_error);
    return (FT_ERR_SUCCESS);
}

ft_bool ft_dual_number::is_thread_safe() const noexcept
{
    this->abort_if_not_initialised("ft_dual_number::is_thread_safe");
    ft_dual_number::set_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

double ft_dual_number::value() const noexcept
{
    int32_t lock_error;
    double result;

    this->abort_if_not_initialised("ft_dual_number::value");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(lock_error);
        return (0.0);
    }
    result = this->_value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    ft_dual_number::set_error(FT_ERR_SUCCESS);
    return (result);
}

double ft_dual_number::derivative() const noexcept
{
    int32_t lock_error;
    double result;

    this->abort_if_not_initialised("ft_dual_number::derivative");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(lock_error);
        return (0.0);
    }
    result = this->_derivative;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    ft_dual_number::set_error(FT_ERR_SUCCESS);
    return (result);
}

ft_dual_number_proxy::ft_dual_number_proxy() noexcept
    : _value(0.0, 0.0)
    , _operation_error(FT_ERR_SUCCESS)
{
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(int32_t error_code) noexcept
    : _value(0.0, 0.0)
    , _operation_error(error_code)
{
    this->_value._operation_error = error_code;
    ft_dual_number::set_error(error_code);
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(const ft_dual_number &value,
    int32_t error_code) noexcept
    : _value(value)
    , _operation_error(error_code)
{
    this->_value._operation_error = error_code;
    ft_dual_number::set_error(error_code);
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(
    const ft_dual_number_proxy &other) noexcept
    : _value(other._value)
    , _operation_error(other._operation_error)
{
    return ;
}

ft_dual_number_proxy::ft_dual_number_proxy(
    ft_dual_number_proxy &&other) noexcept
    : _value(static_cast<ft_dual_number &&>(other._value))
    , _operation_error(other._operation_error)
{
    other._operation_error = FT_ERR_SUCCESS;
    return ;
}

ft_dual_number_proxy::~ft_dual_number_proxy()
{
    return ;
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

    result._operation_error = this->_operation_error;
    ft_dual_number::set_error(this->_operation_error);
    return (result);
}

int32_t ft_dual_number_proxy::get_error() const noexcept
{
    return (this->_operation_error);
}

ft_dual_number_proxy ft_dual_number::operator+(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int32_t initialize_error;
    int32_t lock_error;

    this->abort_if_not_initialised("ft_dual_number::operator+");
    other.abort_if_not_initialised("ft_dual_number::operator+ source");
    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value + other._value;
    result._derivative = this->_derivative + other._derivative;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator-(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int32_t initialize_error;
    int32_t lock_error;

    this->abort_if_not_initialised("ft_dual_number::operator-");
    other.abort_if_not_initialised("ft_dual_number::operator- source");
    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value - other._value;
    result._derivative = this->_derivative - other._derivative;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator*(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int32_t initialize_error;
    int32_t lock_error;

    this->abort_if_not_initialised("ft_dual_number::operator*");
    other.abort_if_not_initialised("ft_dual_number::operator* source");
    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value * other._value;
    result._derivative = this->_value * other._derivative
        + this->_derivative * other._value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number_proxy ft_dual_number::operator/(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    const ft_dual_number *lower;
    const ft_dual_number *upper;
    int32_t initialize_error;
    int32_t lock_error;
    double denominator;
    double epsilon;

    this->abort_if_not_initialised("ft_dual_number::operator/");
    other.abort_if_not_initialised("ft_dual_number::operator/ source");
    if (this->_operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(this->_operation_error);
        return (ft_dual_number_proxy(this->_operation_error));
    }
    if (other._operation_error != FT_ERR_SUCCESS)
    {
        ft_dual_number::set_error(other._operation_error);
        return (ft_dual_number_proxy(other._operation_error));
    }
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    lock_error = ft_dual_number::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    denominator = other._value;
    epsilon = 0.000000000001;
    if (std::fabs(denominator) <= epsilon)
    {
        result._operation_error = FT_ERR_DIVIDE_BY_ZERO;
        ft_dual_number::unlock_pair(lower, upper);
        ft_dual_number::set_error(result._operation_error);
        return (ft_dual_number_proxy(result, result._operation_error));
    }
    result._value = this->_value / denominator;
    result._derivative = (this->_derivative * denominator - this->_value * other._derivative)
        / (denominator * denominator);
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::unlock_pair(lower, upper);
    ft_dual_number::set_error(result._operation_error);
    return (ft_dual_number_proxy(result, result._operation_error));
}

ft_dual_number ft_dual_number::apply_sin() const noexcept
{
    ft_dual_number result;
    int32_t initialize_error;
    int32_t lock_error;
    double value;
    double derivative_value;

    this->abort_if_not_initialised("ft_dual_number::apply_sin");
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    result._value = std::sin(value);
    result._derivative = std::cos(value) * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_cos() const noexcept
{
    ft_dual_number result;
    int32_t initialize_error;
    int32_t lock_error;
    double value;
    double derivative_value;

    this->abort_if_not_initialised("ft_dual_number::apply_cos");
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    result._value = std::cos(value);
    result._derivative = -std::sin(value) * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_exp() const noexcept
{
    ft_dual_number result;
    int32_t initialize_error;
    int32_t lock_error;
    double value;
    double derivative_value;
    double exponential;

    this->abort_if_not_initialised("ft_dual_number::apply_exp");
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    exponential = std::exp(value);
    result._value = exponential;
    result._derivative = exponential * derivative_value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_error(result._operation_error);
    return (result);
}

ft_dual_number ft_dual_number::apply_log() const noexcept
{
    ft_dual_number result;
    int32_t initialize_error;
    int32_t lock_error;
    double value;
    double derivative_value;

    this->abort_if_not_initialised("ft_dual_number::apply_log");
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        result._operation_error = initialize_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        result._operation_error = lock_error;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    value = this->_value;
    derivative_value = this->_derivative;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (value <= 0.0)
    {
        result._operation_error = FT_ERR_INVALID_ARGUMENT;
        ft_dual_number::set_error(result._operation_error);
        return (result);
    }
    result._value = std::log(value);
    result._derivative = derivative_value / value;
    result._operation_error = FT_ERR_SUCCESS;
    ft_dual_number::set_error(result._operation_error);
    return (result);
}


int32_t ft_dual_number::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("ft_dual_number::get_error",
            "called while object is uninitialised");
    return (ft_dual_number::_last_error);
}

const char *ft_dual_number::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("ft_dual_number::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(this->get_error()));
}

static int32_t math_autodiff_prepare_inputs(const ft_vector<double> &point,
    uint64_t active_index, ft_vector<ft_dual_number> &dual_inputs) noexcept
{
    uint64_t dimension;
    uint64_t index;
    int32_t initialize_error;

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
            initialize_error = variable.initialize(value, 1.0);
        else
            initialize_error = variable.initialize(value, 0.0);
        if (initialize_error != FT_ERR_SUCCESS)
            return (FT_ERR_INITIALIZATION_FAILED);
        dual_inputs.push_back(variable);
        index++;
    }
    return (FT_ERR_SUCCESS);
}

int32_t math_autodiff_univariate(math_autodiff_univariate_function function,
    double point, double *value, double *derivative, void *user_data) noexcept
{
    ft_dual_number variable;
    ft_dual_number result;
    int32_t initialize_error;
    int32_t result_error;

    if (function == ft_nullptr || value == ft_nullptr || derivative == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    initialize_error = variable.initialize(point, 1.0);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    result_error = result.initialize(function(variable, user_data));
    if (result_error != FT_ERR_SUCCESS)
        return (result_error);
    *value = result.value();
    if (result.get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    *derivative = result.derivative();
    if (result.get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

int32_t math_autodiff_gradient(math_autodiff_multivariate_function function,
    const ft_vector<double> &point, ft_vector<double> &gradient,
    double *value, void *user_data) noexcept
{
    uint64_t dimension;
    uint64_t index;
    ft_bool value_set;

    if (function == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    dimension = point.size();
    gradient.clear();
    gradient.reserve(dimension);
    index = 0;
    value_set = FT_FALSE;
    while (index < dimension)
    {
        ft_vector<ft_dual_number> dual_inputs;
        ft_dual_number result;
        int32_t result_error;

        if (math_autodiff_prepare_inputs(point, index, dual_inputs) != FT_ERR_SUCCESS)
        {
            gradient.clear();
            return (FT_ERR_INITIALIZATION_FAILED);
        }
        result_error = result.initialize(function(dual_inputs, user_data));
        if (result_error != FT_ERR_SUCCESS)
        {
            gradient.clear();
            return (result_error);
        }
        if (!value_set && value != ft_nullptr)
        {
            *value = result.value();
            if (result.get_error() != FT_ERR_SUCCESS)
            {
                gradient.clear();
                return (FT_ERR_INTERNAL);
            }
            value_set = FT_TRUE;
        }
        gradient.push_back(result.derivative());
        if (result.get_error() != FT_ERR_SUCCESS)
        {
            gradient.clear();
            return (FT_ERR_INTERNAL);
        }
        index++;
    }
    return (FT_ERR_SUCCESS);
}
