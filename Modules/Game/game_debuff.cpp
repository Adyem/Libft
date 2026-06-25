#include "../PThread/pthread_internal.hpp"
#include "game_debuff.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

thread_local int32_t game_debuff::_last_error = FT_ERR_SUCCESS;

int32_t game_debuff::set_error(int32_t error_code) noexcept
{
    game_debuff::_last_error = error_code;
    return (error_code);
}

int32_t game_debuff::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_debuff::get_error");
    return (game_debuff::_last_error);
}

const char *game_debuff::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_debuff::get_error_str");
    return (ft_strerror(game_debuff::_last_error));
}

game_debuff::game_debuff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_debuff::~game_debuff() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_debuff::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_debuff::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_debuff::initialize(const game_debuff &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_debuff::initialize(copy)",
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
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(other.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t game_debuff::initialize(game_debuff &&other) noexcept
{
    return (this->move(other));
}

int32_t game_debuff::move(game_debuff &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_debuff::move",
            "source object is uninitialised");
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
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(other.get_error());
    other._id = 0;
    other._duration = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    other.set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_debuff::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_debuff::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::enable_thread_safety");
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

int32_t game_debuff::disable_thread_safety() noexcept
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

ft_bool game_debuff::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_debuff::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t game_debuff::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t game_debuff::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::lock");
    int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void game_debuff::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_debuff::get_id() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_id");
    return (this->_id);
}

void game_debuff::set_id(int32_t id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_id");
    if (id < 0)
        return ;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_id = id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_debuff::get_duration() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_duration");
    return (this->_duration);
}

void game_debuff::set_duration(int32_t duration) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_duration");
    if (duration < 0)
        return ;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration = duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_debuff::add_duration(int32_t duration) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::add_duration");
    if (duration < 0)
        return ;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration += duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_debuff::sub_duration(int32_t duration) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::sub_duration");
    if (duration < 0)
        return ;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration -= duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_debuff::get_modifier1() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_modifier1");
    return (this->_modifier1);
}

void game_debuff::set_modifier1(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_modifier1");
    this->_modifier1 = mod;
    return ;
}

void game_debuff::add_modifier1(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::add_modifier1");
    this->_modifier1 += mod;
    return ;
}

void game_debuff::sub_modifier1(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::sub_modifier1");
    this->_modifier1 -= mod;
    return ;
}

int32_t game_debuff::get_modifier2() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_modifier2");
    return (this->_modifier2);
}

void game_debuff::set_modifier2(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_modifier2");
    this->_modifier2 = mod;
    return ;
}

void game_debuff::add_modifier2(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::add_modifier2");
    this->_modifier2 += mod;
    return ;
}

void game_debuff::sub_modifier2(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::sub_modifier2");
    this->_modifier2 -= mod;
    return ;
}

int32_t game_debuff::get_modifier3() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_modifier3");
    return (this->_modifier3);
}

void game_debuff::set_modifier3(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_modifier3");
    this->_modifier3 = mod;
    return ;
}

void game_debuff::add_modifier3(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::add_modifier3");
    this->_modifier3 += mod;
    return ;
}

void game_debuff::sub_modifier3(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::sub_modifier3");
    this->_modifier3 -= mod;
    return ;
}

int32_t game_debuff::get_modifier4() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::get_modifier4");
    return (this->_modifier4);
}

void game_debuff::set_modifier4(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::set_modifier4");
    this->_modifier4 = mod;
    return ;
}

void game_debuff::add_modifier4(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::add_modifier4");
    this->_modifier4 += mod;
    return ;
}

void game_debuff::sub_modifier4(int32_t mod) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_debuff::sub_modifier4");
    this->_modifier4 -= mod;
    return ;
}
