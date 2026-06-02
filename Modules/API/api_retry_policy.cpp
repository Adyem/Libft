#include "api.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/basic.hpp"

api_retry_policy::api_retry_policy() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _max_attempts(0), _initial_delay_ms(0), _max_delay_ms(0),
      _backoff_multiplier(0), _circuit_breaker_threshold(0),
      _circuit_breaker_cooldown_ms(0),
      _circuit_breaker_half_open_successes(0), _mutex(ft_nullptr)
{
    return ;
}

api_retry_policy::~api_retry_policy()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

void api_retry_policy::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void api_retry_policy::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t api_retry_policy::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;

    this->abort_if_not_initialised("api_retry_policy::enable_thread_safety");
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

int32_t api_retry_policy::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;

    this->abort_if_not_initialised("api_retry_policy::disable_thread_safety");
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

ft_bool api_retry_policy::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t api_retry_policy::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        this->abort_lifecycle_error("api_retry_policy::initialize",
            "initialize called on initialised instance");
    this->_max_attempts = 0;
    this->_initial_delay_ms = 0;
    this->_max_delay_ms = 0;
    this->_backoff_multiplier = 0;
    this->_circuit_breaker_threshold = 0;
    this->_circuit_breaker_cooldown_ms = 0;
    this->_circuit_breaker_half_open_successes = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_retry_policy::initialize(const api_retry_policy &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_retry_policy::initialize(copy)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (destroy_result);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_max_attempts = other._max_attempts;
    this->_initial_delay_ms = other._initial_delay_ms;
    this->_max_delay_ms = other._max_delay_ms;
    this->_backoff_multiplier = other._backoff_multiplier;
    this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
    this->_circuit_breaker_cooldown_ms = other._circuit_breaker_cooldown_ms;
    this->_circuit_breaker_half_open_successes = other._circuit_breaker_half_open_successes;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_retry_policy::initialize(api_retry_policy &&other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_retry_policy::initialize(move)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (destroy_result);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_max_attempts = other._max_attempts;
    this->_initial_delay_ms = other._initial_delay_ms;
    this->_max_delay_ms = other._max_delay_ms;
    this->_backoff_multiplier = other._backoff_multiplier;
    this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
    this->_circuit_breaker_cooldown_ms = other._circuit_breaker_cooldown_ms;
    this->_circuit_breaker_half_open_successes = other._circuit_breaker_half_open_successes;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._max_attempts = 0;
    other._initial_delay_ms = 0;
    other._max_delay_ms = 0;
    other._backoff_multiplier = 0;
    other._circuit_breaker_threshold = 0;
    other._circuit_breaker_cooldown_ms = 0;
    other._circuit_breaker_half_open_successes = 0;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t api_retry_policy::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    (void)this->disable_thread_safety();
    this->_max_attempts = 0;
    this->_initial_delay_ms = 0;
    this->_max_delay_ms = 0;
    this->_backoff_multiplier = 0;
    this->_circuit_breaker_threshold = 0;
    this->_circuit_breaker_cooldown_ms = 0;
    this->_circuit_breaker_half_open_successes = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t api_retry_policy::move(api_retry_policy &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
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

void api_retry_policy::set_max_attempts(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_max_attempts = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_initial_delay_ms(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_initial_delay_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_max_delay_ms(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_max_delay_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_backoff_multiplier(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_backoff_multiplier = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_threshold(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_threshold = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_cooldown_ms(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_cooldown_ms = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_retry_policy::set_circuit_breaker_half_open_successes(int32_t value) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_circuit_breaker_half_open_successes = value;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t api_retry_policy::get_max_attempts() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_max_attempts;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_initial_delay_ms() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_initial_delay_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_max_delay_ms() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_max_delay_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_backoff_multiplier() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_backoff_multiplier;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_circuit_breaker_threshold() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_circuit_breaker_threshold;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_circuit_breaker_cooldown_ms() const noexcept
{
    int32_t value;

    value = 0;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (0);
    value = this->_circuit_breaker_cooldown_ms;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (value);
}

int32_t api_retry_policy::get_circuit_breaker_half_open_successes() const noexcept
{
    int32_t value;

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
    this->abort_if_not_initialised("api_retry_policy::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
