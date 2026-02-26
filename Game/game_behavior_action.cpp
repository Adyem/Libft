#include "game_behavior_action.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include <new>

thread_local int ft_behavior_action::_last_error = FT_ERR_SUCCESS;
int ft_behavior_action::lock_pair(const ft_behavior_action &first, const ft_behavior_action &second,
        bool *first_locked,
        bool *second_locked)
{
    const ft_behavior_action *ordered_first;
    const ft_behavior_action *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
    {
        return (first.lock_internal(first_locked));
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_behavior_action *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
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
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _mutex(ft_nullptr)
{
    return ;
}

int ft_behavior_action::initialize() noexcept
{
    this->_action_id = 0;
    this->_weight = 0.0;
    this->_cooldown_seconds = 0.0;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_action::initialize(int action_id, double weight,
    double cooldown_seconds) noexcept
{
    this->_action_id = action_id;
    this->_weight = weight;
    this->_cooldown_seconds = cooldown_seconds;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_behavior_action::~ft_behavior_action() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_behavior_action::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_behavior_action::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

int ft_behavior_action::initialize(const ft_behavior_action &other) noexcept
{
    bool other_locked;
    int lock_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    other.unlock_internal(other_locked);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_action::initialize(ft_behavior_action &&other) noexcept
{
    bool other_locked;
    int lock_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    other._action_id = 0;
    other._weight = 0.0;
    other._cooldown_seconds = 0.0;
    other.set_error(FT_ERR_SUCCESS);
    other.unlock_internal(other_locked);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_action::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

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

int ft_behavior_action::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_behavior_action::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_behavior_action::get_action_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int action_id;

    lock_acquired = false;
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

void ft_behavior_action::set_action_id(int action_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
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
    bool lock_acquired;
    int lock_error;
    double weight;

    lock_acquired = false;
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
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
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
    bool lock_acquired;
    int lock_error;
    double cooldown_seconds;

    lock_acquired = false;
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
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
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

int ft_behavior_action::get_error() const noexcept
{
    return (ft_behavior_action::_last_error);
}

const char *ft_behavior_action::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_behavior_action::set_error(int error_code) const noexcept
{
    ft_behavior_action::_last_error = error_code;
    return ;
}
