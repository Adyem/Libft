#include "../PThread/pthread_internal.hpp"
#include "game_behavior_action.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t ft_behavior_action::_last_error = FT_ERR_SUCCESS;
int32_t ft_behavior_action::lock_pair(const ft_behavior_action &first, const ft_behavior_action &second,
        ft_bool *first_locked,
        ft_bool *second_locked)
{
    const ft_behavior_action *ordered_first;
    const ft_behavior_action *ordered_second;
    int32_t lock_error;
    ft_bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = FT_FALSE;
    if (second_locked != ft_nullptr)
        *second_locked = FT_FALSE;
    if (&first == &second)
    {
        return (first.lock_internal(first_locked));
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = FT_FALSE;
    if (ordered_first > ordered_second)
    {
        const ft_behavior_action *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = FT_TRUE;
    }
    while (FT_TRUE)
    {
        ft_bool lower_locked;
        ft_bool upper_locked;

        lower_locked = FT_FALSE;
        upper_locked = FT_FALSE;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        pt_thread_sleep(1);
    }
}

ft_behavior_action::ft_behavior_action() noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED), _mutex(ft_nullptr)
{
    return ;
}

ft_behavior_action::ft_behavior_action(const ft_behavior_action &other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED), _mutex(ft_nullptr)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_behavior_action::ft_behavior_action(copy)",
            "source object is not initialised");
    }
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_behavior_action::ft_behavior_action(ft_behavior_action &&other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED), _mutex(ft_nullptr)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_behavior_action::ft_behavior_action(move)",
            "source object is not initialised");
    }
    if (this->initialize(static_cast<ft_behavior_action &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t ft_behavior_action::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "ft_behavior_action::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_action_id = 0;
    this->_weight = 0.0;
    this->_cooldown_seconds = 0.0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_behavior_action::initialize(int32_t action_id, double weight,
    double cooldown_seconds) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "ft_behavior_action::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_action_id = action_id;
    this->_weight = weight;
    this->_cooldown_seconds = cooldown_seconds;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_behavior_action::~ft_behavior_action() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t ft_behavior_action::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void ft_behavior_action::unlock_internal(ft_bool lock_acquired) const noexcept
{

    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_behavior_action::initialize(const ft_behavior_action &other) noexcept
{
    ft_bool other_locked;
    int32_t lock_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_behavior_action::initialize(copy)",
            "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    other_locked = FT_FALSE;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other.unlock_internal(other_locked);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_behavior_action::initialize(ft_behavior_action &&other) noexcept
{
    ft_bool other_locked;
    int32_t lock_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_behavior_action::initialize(move)",
            "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    other_locked = FT_FALSE;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._action_id = 0;
    other._weight = 0.0;
    other._cooldown_seconds = 0.0;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    other.set_error(FT_ERR_SUCCESS);
    other.unlock_internal(other_locked);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_behavior_action::move(ft_behavior_action &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    return (this->initialize(static_cast<ft_behavior_action &&>(other)));
}

int32_t ft_behavior_action::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_action_id = 0;
    this->_weight = 0.0;
    this->_cooldown_seconds = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_behavior_action::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t ft_behavior_action::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_behavior_action::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_behavior_action::get_action_id() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t action_id;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(lock_error);
        return (0);
    }
    action_id = this->_action_id;
    const_cast<ft_behavior_action *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (action_id);
}

void ft_behavior_action::set_action_id(int32_t action_id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_action_id = action_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

double ft_behavior_action::get_weight() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    double weight;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(lock_error);
        return (0.0);
    }
    weight = this->_weight;
    const_cast<ft_behavior_action *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (weight);
}

void ft_behavior_action::set_weight(double weight) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_weight = weight;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

double ft_behavior_action::get_cooldown_seconds() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    double cooldown_seconds;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(lock_error);
        return (0.0);
    }
    cooldown_seconds = this->_cooldown_seconds;
    const_cast<ft_behavior_action *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (cooldown_seconds);
}

void ft_behavior_action::set_cooldown_seconds(double cooldown_seconds) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_cooldown_seconds = cooldown_seconds;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int32_t ft_behavior_action::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_behavior_action::get_error");
    return (ft_behavior_action::_last_error);
}

const char *ft_behavior_action::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_behavior_action::get_error_str");
    return (ft_strerror(ft_behavior_action::_last_error));
}

int32_t ft_behavior_action::set_error(int32_t error_code) noexcept
{
    ft_behavior_action::_last_error = error_code;
    return (error_code);
}
