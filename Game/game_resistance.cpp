#include "../PThread/pthread_internal.hpp"
#include "game_resistance.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local uint32_t game_resistance::_last_error = FT_ERR_SUCCESS;

game_resistance::game_resistance() noexcept
    : _percent_value(0), _flat_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_resistance::game_resistance(const game_resistance &other) noexcept
    : _percent_value(0), _flat_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_resistance::game_resistance(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

game_resistance::game_resistance(game_resistance &&other) noexcept
    : _percent_value(0), _flat_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_resistance::game_resistance(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

game_resistance::~game_resistance() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_resistance::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_resistance::initialize", "already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_percent_value = 0;
    this->_flat_value = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::initialize(const game_resistance &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_resistance::initialize(copy)",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(other.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::initialize(game_resistance &&other) noexcept
{
    return (this->move(other));
}

int32_t game_resistance::move(game_resistance &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_resistance::move", "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(other.get_error());
    other._percent_value = 0;
    other._flat_value = 0;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    other.set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_percent_value = 0;
    this->_flat_value = 0;
    if (this->_mutex != ft_nullptr)
        (void)this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t game_resistance::unlock_internal(ft_bool lock_acquired) const noexcept
{

    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::set_percent(int32_t percent_value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::set_percent");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_percent_value = percent_value;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::set_flat(int32_t flat_value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::set_flat");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::set_values(int32_t percent_value, int32_t flat_value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::set_values");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_percent_value = percent_value;
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::reset() noexcept
{
    return (this->set_values(0, 0));
}

int32_t game_resistance::get_percent() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t percent_value;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::get_percent");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    percent_value = this->_percent_value;
    const_cast<game_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (percent_value);
}

int32_t game_resistance::get_flat() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t flat_value;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::get_flat");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    flat_value = this->_flat_value;
    const_cast<game_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (flat_value);
}

int32_t game_resistance::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_resistance::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_resistance::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_resistance::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_resistance::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_resistance::get_error");
    return (game_resistance::_last_error);
}

const char *game_resistance::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_resistance::get_error_str");
    return (ft_strerror(game_resistance::_last_error));
}

uint32_t game_resistance::set_error(uint32_t error_code) noexcept
{
    game_resistance::_last_error = error_code;
    return (error_code);
}
