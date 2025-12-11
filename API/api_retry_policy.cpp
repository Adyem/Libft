#include "api.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void api_retry_policy_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void api_retry_policy::set_error(int error) const noexcept
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int api_retry_policy::lock_pair(const api_retry_policy &first,
    const api_retry_policy &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const api_retry_policy *ordered_first;
    const api_retry_policy *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const api_retry_policy *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        api_retry_policy_sleep_backoff();
    }
}

api_retry_policy::api_retry_policy() noexcept
    : _max_attempts(0), _initial_delay_ms(0), _max_delay_ms(0),
      _backoff_multiplier(0), _circuit_breaker_threshold(0),
      _circuit_breaker_cooldown_ms(0),
      _circuit_breaker_half_open_successes(0), _error_code(FT_ERR_SUCCESSS),
      _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

api_retry_policy::api_retry_policy(const api_retry_policy &other) noexcept
    : _max_attempts(0), _initial_delay_ms(0), _max_delay_ms(0),
      _backoff_multiplier(0), _circuit_breaker_threshold(0),
      _circuit_breaker_cooldown_ms(0),
      _circuit_breaker_half_open_successes(0), _error_code(FT_ERR_SUCCESSS),
      _mutex()
{
    {
        ft_unique_lock<pt_mutex> other_guard(other._mutex);

        if (other_guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(other_guard.get_error());
            return ;
        }
        this->_max_attempts = other._max_attempts;
        this->_initial_delay_ms = other._initial_delay_ms;
        this->_max_delay_ms = other._max_delay_ms;
        this->_backoff_multiplier = other._backoff_multiplier;
        this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
        this->_circuit_breaker_cooldown_ms =
            other._circuit_breaker_cooldown_ms;
        this->_circuit_breaker_half_open_successes =
            other._circuit_breaker_half_open_successes;
        this->_error_code = other._error_code;
        this->set_error(other._error_code);
    }
    return ;
}

api_retry_policy &api_retry_policy::operator=(
    const api_retry_policy &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = api_retry_policy::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_max_attempts = other._max_attempts;
    this->_initial_delay_ms = other._initial_delay_ms;
    this->_max_delay_ms = other._max_delay_ms;
    this->_backoff_multiplier = other._backoff_multiplier;
    this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
    this->_circuit_breaker_cooldown_ms =
        other._circuit_breaker_cooldown_ms;
    this->_circuit_breaker_half_open_successes =
        other._circuit_breaker_half_open_successes;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return (*this);
}

api_retry_policy::api_retry_policy(api_retry_policy &&other) noexcept
    : _max_attempts(0), _initial_delay_ms(0), _max_delay_ms(0),
      _backoff_multiplier(0), _circuit_breaker_threshold(0),
      _circuit_breaker_cooldown_ms(0),
      _circuit_breaker_half_open_successes(0), _error_code(FT_ERR_SUCCESSS),
      _mutex()
{
    {
        ft_unique_lock<pt_mutex> other_guard(other._mutex);

        if (other_guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(other_guard.get_error());
            return ;
        }
        this->_max_attempts = other._max_attempts;
        this->_initial_delay_ms = other._initial_delay_ms;
        this->_max_delay_ms = other._max_delay_ms;
        this->_backoff_multiplier = other._backoff_multiplier;
        this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
        this->_circuit_breaker_cooldown_ms =
            other._circuit_breaker_cooldown_ms;
        this->_circuit_breaker_half_open_successes =
            other._circuit_breaker_half_open_successes;
        this->_error_code = other._error_code;
        other._max_attempts = 0;
        other._initial_delay_ms = 0;
        other._max_delay_ms = 0;
        other._backoff_multiplier = 0;
        other._circuit_breaker_threshold = 0;
        other._circuit_breaker_cooldown_ms = 0;
        other._circuit_breaker_half_open_successes = 0;
        other._error_code = FT_ERR_SUCCESSS;
        this->set_error(this->_error_code);
        other.set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

api_retry_policy &api_retry_policy::operator=(api_retry_policy &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = api_retry_policy::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_max_attempts = other._max_attempts;
    this->_initial_delay_ms = other._initial_delay_ms;
    this->_max_delay_ms = other._max_delay_ms;
    this->_backoff_multiplier = other._backoff_multiplier;
    this->_circuit_breaker_threshold = other._circuit_breaker_threshold;
    this->_circuit_breaker_cooldown_ms =
        other._circuit_breaker_cooldown_ms;
    this->_circuit_breaker_half_open_successes =
        other._circuit_breaker_half_open_successes;
    this->_error_code = other._error_code;
    other._max_attempts = 0;
    other._initial_delay_ms = 0;
    other._max_delay_ms = 0;
    other._backoff_multiplier = 0;
    other._circuit_breaker_threshold = 0;
    other._circuit_breaker_cooldown_ms = 0;
    other._circuit_breaker_half_open_successes = 0;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return (*this);
}

api_retry_policy::~api_retry_policy()
{
    return ;
}

void api_retry_policy::reset() noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_max_attempts = 0;
        this->_initial_delay_ms = 0;
        this->_max_delay_ms = 0;
        this->_backoff_multiplier = 0;
        this->_circuit_breaker_threshold = 0;
        this->_circuit_breaker_cooldown_ms = 0;
        this->_circuit_breaker_half_open_successes = 0;
        this->_error_code = FT_ERR_SUCCESSS;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_max_attempts(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_max_attempts = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_initial_delay_ms(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_initial_delay_ms = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_max_delay_ms(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_max_delay_ms = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_backoff_multiplier(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_backoff_multiplier = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_circuit_breaker_threshold(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_circuit_breaker_threshold = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_circuit_breaker_cooldown_ms(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_circuit_breaker_cooldown_ms = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

void api_retry_policy::set_circuit_breaker_half_open_successes(int value) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_circuit_breaker_half_open_successes = value;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return ;
}

int api_retry_policy::get_max_attempts() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_max_attempts;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_initial_delay_ms() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_initial_delay_ms;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_max_delay_ms() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_max_delay_ms;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_backoff_multiplier() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_backoff_multiplier;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_circuit_breaker_threshold() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_circuit_breaker_threshold;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_circuit_breaker_cooldown_ms() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_circuit_breaker_cooldown_ms;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_circuit_breaker_half_open_successes() const noexcept
{
    int value;

    value = 0;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            return (0);
        }
        value = this->_circuit_breaker_half_open_successes;
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (value);
}

int api_retry_policy::get_error() const noexcept
{
    return (this->_error_code);
}

const char *api_retry_policy::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
