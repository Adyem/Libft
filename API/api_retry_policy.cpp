#include "api.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

api_retry_policy::api_retry_policy() noexcept
    : _initialized_state(api_retry_policy::_state_uninitialized),
      _max_attempts(0), _initial_delay_ms(0), _max_delay_ms(0),
      _backoff_multiplier(0), _circuit_breaker_threshold(0),
      _circuit_breaker_cooldown_ms(0),
      _circuit_breaker_half_open_successes(0), _mutex(ft_nullptr)
{
    return ;
}

api_retry_policy::~api_retry_policy()
{
    if (this->_initialized_state == api_retry_policy::_state_initialized)
        (void)this->destroy();
    return ;
}

void api_retry_policy::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "api_retry_policy lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void api_retry_policy::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == api_retry_policy::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int api_retry_policy::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int initialize_result;

    this->abort_if_not_initialized("api_retry_policy::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_result = new_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (initialize_result);
    }
    this->_mutex = new_mutex;
    return (FT_ERR_SUCCESS);
}

int api_retry_policy::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int destroy_result;

    this->abort_if_not_initialized("api_retry_policy::disable_thread_safety");
    mutex_pointer = this->_mutex;
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    this->_mutex = ft_nullptr;
    destroy_result = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_result != FT_ERR_SUCCESS)
        return (destroy_result);
    return (FT_ERR_SUCCESS);
}

bool api_retry_policy::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int api_retry_policy::initialize() noexcept
{
    if (this->_initialized_state == api_retry_policy::_state_initialized)
        this->abort_lifecycle_error("api_retry_policy::initialize",
            "initialize called on initialized instance");
    this->_max_attempts = 0;
    this->_initial_delay_ms = 0;
    this->_max_delay_ms = 0;
    this->_backoff_multiplier = 0;
    this->_circuit_breaker_threshold = 0;
    this->_circuit_breaker_cooldown_ms = 0;
    this->_circuit_breaker_half_open_successes = 0;
    this->_initialized_state = api_retry_policy::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int api_retry_policy::destroy() noexcept
{
    if (this->_initialized_state != api_retry_policy::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_max_attempts = 0;
    this->_initial_delay_ms = 0;
    this->_max_delay_ms = 0;
    this->_backoff_multiplier = 0;
    this->_circuit_breaker_threshold = 0;
    this->_circuit_breaker_cooldown_ms = 0;
    this->_circuit_breaker_half_open_successes = 0;
    (void)this->disable_thread_safety();
    this->_initialized_state = api_retry_policy::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

void api_retry_policy::reset() noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_max_attempts = 0;
    this->_initial_delay_ms = 0;
    this->_max_delay_ms = 0;
    this->_backoff_multiplier = 0;
    this->_circuit_breaker_threshold = 0;
    this->_circuit_breaker_cooldown_ms = 0;
    this->_circuit_breaker_half_open_successes = 0;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_max_attempts(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_max_attempts = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_initial_delay_ms(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_initial_delay_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_max_delay_ms(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_max_delay_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_backoff_multiplier(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_backoff_multiplier = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_threshold(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_threshold = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_cooldown_ms(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_cooldown_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_half_open_successes(int value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_half_open_successes = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int api_retry_policy::get_max_attempts() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_max_attempts;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_initial_delay_ms() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_initial_delay_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_max_delay_ms() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_max_delay_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_backoff_multiplier() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_backoff_multiplier;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_circuit_breaker_threshold() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_circuit_breaker_threshold;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_circuit_breaker_cooldown_ms() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_circuit_breaker_cooldown_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int api_retry_policy::get_circuit_breaker_half_open_successes() const noexcept
{
    int value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_circuit_breaker_half_open_successes;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *api_retry_policy::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("api_retry_policy::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
