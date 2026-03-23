#include "../PThread/pthread_internal.hpp"
#include "game_reputation.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"

thread_local int32_t game_reputation::_last_error = FT_ERR_SUCCESS;

int32_t game_reputation::set_error(int32_t error_code) noexcept
{
    game_reputation::_last_error = error_code;
    return (error_code);
}

int32_t game_reputation::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_reputation::get_error");
    return (static_cast<int32_t>(game_reputation::_last_error));
}

const char *game_reputation::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_reputation::get_error_str");
    return (ft_strerror(game_reputation::_last_error));
}

game_reputation::game_reputation() noexcept
    : _milestones(), _reps(), _total_rep(0), _current_rep(0),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_reputation::game_reputation(const game_reputation &other) noexcept
    : _milestones(), _reps(), _total_rep(0), _current_rep(0),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_reputation::game_reputation(copy)",
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

game_reputation::game_reputation(game_reputation &&other) noexcept
    : _milestones(), _reps(), _total_rep(0), _current_rep(0),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_reputation::game_reputation(move)",
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

game_reputation::~game_reputation() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_reputation::initialize() noexcept
{
    int32_t milestones_error;
    int32_t reps_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_reputation::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    milestones_error = this->_milestones.initialize();
    if (milestones_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(milestones_error);
        return (milestones_error);
    }
    reps_error = this->_reps.initialize();
    if (reps_error != FT_ERR_SUCCESS)
    {
        (void)this->_milestones.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(reps_error);
        return (reps_error);
    }
    this->_total_rep = 0;
    this->_current_rep = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::initialize(const ft_map<int32_t, int32_t> &milestones,
    int32_t total) noexcept
{
    int32_t initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_milestones = milestones;
    this->_total_rep = total;
    this->_current_rep = 0;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::initialize(const game_reputation &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_reputation::initialize(copy)",
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
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_milestones = other._milestones;
    this->_reps = other._reps;
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::initialize(game_reputation &&other) noexcept
{
    return (this->move(other));
}

int32_t game_reputation::move(game_reputation &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_reputation::move",
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
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->get_error());
    }
    this->_milestones = other._milestones;
    this->_reps = other._reps;
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->set_error(other.get_error());
    (void)other.destroy();
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::destroy() noexcept
{
    int32_t milestones_error;
    int32_t reps_error;
    int32_t disable_error;
    int32_t first_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    first_error = FT_ERR_SUCCESS;
    milestones_error = this->_milestones.destroy();
    if (first_error == FT_ERR_SUCCESS && milestones_error != FT_ERR_SUCCESS)
        first_error = milestones_error;
    reps_error = this->_reps.destroy();
    if (first_error == FT_ERR_SUCCESS && reps_error != FT_ERR_SUCCESS)
        first_error = reps_error;
    this->_total_rep = 0;
    this->_current_rep = 0;
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (first_error != FT_ERR_SUCCESS)
    {
        this->set_error(first_error);
        return (first_error);
    }
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_reputation::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::enable_thread_safety");
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

int32_t game_reputation::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_reputation::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_reputation::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::unlock_internal(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::unlock_internal");
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_reputation::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::lock");
    return (this->lock_internal(lock_acquired));
}

void game_reputation::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_reputation::get_total_rep() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t total;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_total_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    total = this->_total_rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (total);
}

void game_reputation::set_total_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_total_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (rep >= 0);
    if (valid)
        this->_total_rep = rep;
    (void)this->unlock_internal(lock_acquired);
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    return ;
}

void game_reputation::add_total_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::add_total_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_total_rep += rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_reputation::sub_total_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::sub_total_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_total_rep -= rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_reputation::get_current_rep() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t current;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_current_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    current = this->_current_rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (current);
}

void game_reputation::set_current_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_current_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (rep >= 0);
    if (valid)
        this->_current_rep = rep;
    (void)this->unlock_internal(lock_acquired);
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    return ;
}

void game_reputation::add_current_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::add_current_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_rep += rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_reputation::sub_current_rep(int32_t rep) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::sub_current_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_rep -= rep;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int32_t, int32_t> &game_reputation::get_milestones() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_milestones");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_milestones);
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_milestones);
}

const ft_map<int32_t, int32_t> &game_reputation::get_milestones() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_milestones const");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_milestones);
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_milestones);
}

void game_reputation::set_milestones(const ft_map<int32_t, int32_t> &milestones) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_milestones");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_milestones = milestones;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_reputation::get_milestone(int32_t id) const noexcept
{
    const Pair<int32_t, int32_t> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_milestone");
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    value = entry->value;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void game_reputation::set_milestone(int32_t id, int32_t value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    Pair<int32_t, int32_t> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_milestone");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
        this->_milestones.insert(id, value);
    else
        entry->value = value;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int32_t, int32_t> &game_reputation::get_reps() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_reps");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_reps);
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_reps);
}

const ft_map<int32_t, int32_t> &game_reputation::get_reps() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_reps const");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_reps);
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_reps);
}

void game_reputation::set_reps(const ft_map<int32_t, int32_t> &reps) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_reps");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_reps = reps;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_reputation::get_rep(int32_t id) const noexcept
{
    const Pair<int32_t, int32_t> *entry;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::get_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    value = entry->value;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void game_reputation::set_rep(int32_t id, int32_t value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    Pair<int32_t, int32_t> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_reputation::set_rep");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
        this->_reps.insert(id, value);
    else
        entry->value = value;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}
