#include "../PThread/pthread_internal.hpp"
#include "game_achievement.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"

thread_local int32_t ft_goal::_last_error = FT_ERR_SUCCESS;
thread_local int32_t ft_achievement::_last_error = FT_ERR_SUCCESS;

int32_t ft_goal::set_error(int32_t error_code) noexcept
{
    ft_goal::_last_error = error_code;
    return (error_code);
}

int32_t ft_goal::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_goal::get_error");
    return (ft_goal::_last_error);
}

const char *ft_goal::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_goal::get_error_str");
    return (ft_strerror(this->get_error()));
}

int32_t ft_achievement::set_error(int32_t error_code) noexcept
{
    ft_achievement::_last_error = error_code;
    return (error_code);
}

int32_t ft_achievement::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_achievement::get_error");
    return (ft_achievement::_last_error);
}

const char *ft_achievement::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_achievement::get_error_str");
    return (ft_strerror(this->get_error()));
}

ft_goal::ft_goal() noexcept
    : _target(0), _progress(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_goal::ft_goal(const ft_goal &other) noexcept
    : _target(0), _progress(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_goal::ft_goal(copy)",
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

ft_goal::ft_goal(ft_goal &&other) noexcept
    : _target(0), _progress(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_goal::ft_goal(move)",
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

ft_goal::~ft_goal() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_goal::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_goal::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_target = 0;
    this->_progress = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_goal::initialize(const ft_goal &other) noexcept
{
    int32_t initialize_error;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_goal::initialize(copy)", "source object is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_target = other._target;
    this->_progress = other._progress;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_goal::initialize(ft_goal &&other) noexcept
{
    const int32_t initialize_error = this->initialize(static_cast<const ft_goal &>(other));

    this->set_error(initialize_error);
    return (initialize_error);
}

int32_t ft_goal::move(ft_goal &other) noexcept
{
    return (this->initialize(static_cast<ft_goal &&>(other)));
}

int32_t ft_goal::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_target = 0;
    this->_progress = 0;
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_goal::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::enable_thread_safety");
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

int32_t ft_goal::disable_thread_safety() noexcept
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

ft_bool ft_goal::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_goal::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::lock_internal");
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

int32_t ft_goal::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_goal::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_goal::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::unlock");
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}

int32_t ft_goal::get_target() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t target_value;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::get_target");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    target_value = this->_target;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (target_value);
}

void ft_goal::set_target(int32_t target) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::set_target");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_target = target;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_goal::get_progress() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t progress_value;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::get_progress");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    progress_value = this->_progress;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (progress_value);
}

void ft_goal::set_progress(int32_t value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::set_progress");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_progress = value;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_goal::add_progress(int32_t delta) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_goal::add_progress");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_progress += delta;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return ;
}


ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_achievement::ft_achievement(const ft_achievement &other) noexcept
    : _id(0), _goals(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_achievement::ft_achievement(copy)",
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

ft_achievement::ft_achievement(ft_achievement &&other) noexcept
    : _id(0), _goals(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_achievement::ft_achievement(move)",
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

ft_achievement::~ft_achievement() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_achievement::initialize() noexcept
{
    int32_t goals_initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_achievement::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    goals_initialize_error = this->_goals.initialize();
    if (goals_initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(goals_initialize_error);
        return (goals_initialize_error);
    }
    this->_goals.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_achievement::initialize(const ft_achievement &other) noexcept
{
    int32_t initialize_error;
    const Pair<int32_t, ft_goal> *goal_entry;
    const Pair<int32_t, ft_goal> *goals_end;
    ft_goal                   new_goal;
    ft_size_t                    goals_count;
    ft_size_t                    index;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_achievement::initialize(copy)", "source object is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_id = other._id;
    goals_count = other._goals.size();
    goals_end = other._goals.end();
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(FT_ERR_INVALID_STATE);
            return (FT_ERR_INVALID_STATE);
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_achievement::initialize(ft_achievement &&other) noexcept
{
    int32_t initialize_error;
    const Pair<int32_t, ft_goal> *goal_entry;
    const Pair<int32_t, ft_goal> *goals_end;
    ft_goal                   new_goal;
    ft_size_t                    goals_count;
    ft_size_t                    index;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_achievement::initialize(move)", "source object is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_id = other._id;
    goals_count = other._goals.size();
    goals_end = other._goals.end();
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(FT_ERR_INVALID_STATE);
            return (FT_ERR_INVALID_STATE);
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_achievement::move(ft_achievement &other) noexcept
{
    return (this->initialize(static_cast<ft_achievement &&>(other)));
}

int32_t ft_achievement::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_id = 0;
    this->_goals.clear();
    (void)this->_goals.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_achievement::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::enable_thread_safety");
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

int32_t ft_achievement::disable_thread_safety() noexcept
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

ft_bool ft_achievement::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_achievement::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::lock_internal");
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

int32_t ft_achievement::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_achievement::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_achievement::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::unlock");
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}

int32_t ft_achievement::get_id() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t identifier;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::get_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = this->_id;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (identifier);
}

void ft_achievement::set_id(int32_t id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::set_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_id = id;
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int32_t, ft_goal> &ft_achievement::get_goals() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::get_goals");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_goals);
    }
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (this->_goals);
}

const ft_map<int32_t, ft_goal> &ft_achievement::get_goals() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::get_goals const");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_goals);
    }
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (this->_goals);
}

void ft_achievement::set_goals(const ft_map<int32_t, ft_goal> &goals) noexcept
{
    ft_bool lock_acquired;
    const Pair<int32_t, ft_goal> *goal_entry;
    const Pair<int32_t, ft_goal> *goals_end;
    ft_goal                   new_goal;
    ft_size_t                    goals_count;
    ft_size_t                    index;
    int32_t                       lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::set_goals");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_goals.clear();
    goals_count = goals.size();
    goals_end = goals.end();
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_achievement::get_goal(int32_t id) const noexcept
{
    const Pair<int32_t, ft_goal> *entry;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t target_value;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::get_goal");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    target_value = entry->value.get_target();
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (target_value);
}

void ft_achievement::set_goal(int32_t id, int32_t goal) noexcept
{
    Pair<int32_t, ft_goal> *entry;
    ft_goal             new_goal;
    ft_bool                lock_acquired;
    int32_t                 lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::set_goal");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(goal);
        new_goal.set_progress(0);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.set_target(goal);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_achievement::get_progress(int32_t id) const noexcept
{
    const Pair<int32_t, ft_goal> *entry;
    ft_bool                       lock_acquired;
    int32_t                        lock_error;
    int32_t                        progress_value;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::get_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    progress_value = entry->value.get_progress();
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (progress_value);
}

void ft_achievement::set_progress(int32_t id, int32_t progress) noexcept
{
    Pair<int32_t, ft_goal> *entry;
    ft_goal             new_goal;
    ft_bool                lock_acquired;
    int32_t                 lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::set_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(0);
        new_goal.set_progress(progress);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.set_progress(progress);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_achievement::add_progress(int32_t id, int32_t value) noexcept
{
    Pair<int32_t, ft_goal> *entry;
    ft_goal             new_goal;
    ft_bool                lock_acquired;
    int32_t                 lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::add_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(0);
        new_goal.set_progress(value);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.add_progress(value);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_bool ft_achievement::is_goal_complete(int32_t id) const noexcept
{
    const Pair<int32_t, ft_goal> *entry;

    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool is_complete_flag;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::is_goal_complete");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (FT_FALSE);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_FALSE);
    }
    is_complete_flag = (entry->value.get_progress() >= entry->value.get_target());
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (is_complete_flag);
}

ft_bool ft_achievement::is_complete() const noexcept
{
    const Pair<int32_t, ft_goal> *goal_entry;
    const Pair<int32_t, ft_goal> *goals_end;
    ft_size_t                    goals_count;
    ft_size_t                    index;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_achievement::is_complete");
    goals_count = this->_goals.size();
    goals_end = this->_goals.end();
    if (goals_count == 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (FT_FALSE);
    }
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (goal_entry->value.get_progress() < goal_entry->value.get_target())
        {
            (void)this->unlock_internal(lock_acquired);
            this->set_error(FT_ERR_SUCCESS);
            return (FT_FALSE);
        }
        goal_entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
