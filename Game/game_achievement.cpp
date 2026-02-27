#include "../PThread/pthread_internal.hpp"
#include "game_achievement.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

thread_local int ft_goal::_last_error = FT_ERR_SUCCESS;
thread_local int ft_achievement::_last_error = FT_ERR_SUCCESS;

void ft_goal::set_error(int error_code) const noexcept
{
    ft_goal::_last_error = error_code;
    return ;
}

int ft_goal::get_error() const noexcept
{
    return (ft_goal::_last_error);
}

const char *ft_goal::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_achievement::set_error(int error_code) const noexcept
{
    ft_achievement::_last_error = error_code;
    return ;
}

int ft_achievement::get_error() const noexcept
{
    return (ft_achievement::_last_error);
}

const char *ft_achievement::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_goal::ft_goal() noexcept
    : _target(0), _progress(0), _mutex(ft_nullptr),
      _initialized_state(ft_goal::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_goal::~ft_goal() noexcept
{
    if (this->_initialized_state == ft_goal::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_goal::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_goal lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_goal::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_goal::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_goal::initialize() noexcept
{
    if (this->_initialized_state == ft_goal::_state_initialized)
    {
        this->abort_lifecycle_error("ft_goal::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_target = 0;
    this->_progress = 0;
    this->_initialized_state = ft_goal::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_goal::initialize(const ft_goal &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_goal::_state_initialized)
    {
        other.abort_lifecycle_error("ft_goal::initialize(copy)",
            "source object is not initialized");
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

int ft_goal::initialize(ft_goal &&other) noexcept
{
    const int initialize_error = this->initialize(static_cast<const ft_goal &>(other));

    this->set_error(initialize_error);
    return (initialize_error);
}

int ft_goal::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_goal::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_target = 0;
    this->_progress = 0;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_goal::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_goal::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_goal::enable_thread_safety");
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

int ft_goal::disable_thread_safety() noexcept
{
    int destroy_error;

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

bool ft_goal::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_goal::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_goal::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_goal::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_goal::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_goal::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_goal::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_goal::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_goal::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_goal::get_target() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int target_value;

    this->abort_if_not_initialized("ft_goal::get_target");
    lock_acquired = false;
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

void ft_goal::set_target(int target) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_goal::set_target");
    lock_acquired = false;
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

int ft_goal::get_progress() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int progress_value;

    this->abort_if_not_initialized("ft_goal::get_progress");
    lock_acquired = false;
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

void ft_goal::set_progress(int value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_goal::set_progress");
    lock_acquired = false;
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

void ft_goal::add_progress(int delta) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_goal::add_progress");
    lock_acquired = false;
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

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_goal::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_goal::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _mutex(ft_nullptr),
      _initialized_state(ft_achievement::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_achievement::~ft_achievement() noexcept
{
    if (this->_initialized_state == ft_achievement::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_achievement::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_achievement::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_achievement lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_achievement::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_achievement::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_achievement::initialize() noexcept
{
    int goals_initialize_error;

    if (this->_initialized_state == ft_achievement::_state_initialized)
    {
        this->abort_lifecycle_error("ft_achievement::initialize",
            "called while object is already initialized");
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
    this->_initialized_state = ft_achievement::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_achievement::initialize(const ft_achievement &other) noexcept
{
    int initialize_error;
    const Pair<int, ft_goal> *goal_entry;
    const Pair<int, ft_goal> *goals_end;
    ft_goal                   new_goal;
    size_t                    goals_count;
    size_t                    index;

    if (other._initialized_state != ft_achievement::_state_initialized)
    {
        other.abort_lifecycle_error("ft_achievement::initialize(copy)",
            "source object is not initialized");
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
            this->_initialized_state = ft_achievement::_state_destroyed;
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

int ft_achievement::initialize(ft_achievement &&other) noexcept
{
    int initialize_error;
    const Pair<int, ft_goal> *goal_entry;
    const Pair<int, ft_goal> *goals_end;
    ft_goal                   new_goal;
    size_t                    goals_count;
    size_t                    index;

    if (other._initialized_state != ft_achievement::_state_initialized)
    {
        other.abort_lifecycle_error("ft_achievement::initialize(move)",
            "source object is not initialized");
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
            this->_initialized_state = ft_achievement::_state_destroyed;
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

int ft_achievement::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_achievement::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_goals.clear();
    (void)this->_goals.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_achievement::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_achievement::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_achievement::enable_thread_safety");
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

int ft_achievement::disable_thread_safety() noexcept
{
    int destroy_error;

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

bool ft_achievement::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_achievement::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_achievement::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_achievement::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_achievement::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_achievement::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_achievement::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_achievement::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_achievement::get_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_achievement::get_id");
    lock_acquired = false;
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

void ft_achievement::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_achievement::set_id");
    lock_acquired = false;
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

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_achievement::get_goals");
    lock_acquired = false;
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

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_achievement::get_goals const");
    lock_acquired = false;
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

void ft_achievement::set_goals(const ft_map<int, ft_goal> &goals) noexcept
{
    bool lock_acquired;
    const Pair<int, ft_goal> *goal_entry;
    const Pair<int, ft_goal> *goals_end;
    ft_goal                   new_goal;
    size_t                    goals_count;
    size_t                    index;
    int                       lock_error;
    int                       unlock_error;

    this->abort_if_not_initialized("ft_achievement::set_goals");
    lock_acquired = false;
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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error == FT_ERR_SUCCESS)
                this->set_error(FT_ERR_INVALID_STATE);
            else
                this->set_error(unlock_error);
            this->_initialized_state = ft_achievement::_state_destroyed;
            return ;
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    int target_value;

    this->abort_if_not_initialized("ft_achievement::get_goal");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (unlock_error);
        }
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    target_value = entry->value.get_target();
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (unlock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (target_value);
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;
    bool                lock_acquired;
    int                 lock_error;
    int                 unlock_error;

    this->abort_if_not_initialized("ft_achievement::set_goal");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                this->set_error(unlock_error);
                return ;
            }
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(goal);
        new_goal.set_progress(0);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.set_target(goal);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;
    bool                       lock_acquired;
    int                        lock_error;
    int                        unlock_error;
    int                        progress_value;

    this->abort_if_not_initialized("ft_achievement::get_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (0);
        }
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    progress_value = entry->value.get_progress();
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (progress_value);
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;
    bool                lock_acquired;
    int                 lock_error;
    int                 unlock_error;

    this->abort_if_not_initialized("ft_achievement::set_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                this->set_error(unlock_error);
                return ;
            }
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(0);
        new_goal.set_progress(progress);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.set_progress(progress);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;
    bool                lock_acquired;
    int                 lock_error;
    int                 unlock_error;

    this->abort_if_not_initialized("ft_achievement::add_progress");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
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
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                this->set_error(unlock_error);
                return ;
            }
            this->set_error(FT_ERR_INVALID_STATE);
            return ;
        }
        new_goal.set_target(0);
        new_goal.set_progress(value);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    entry->value.add_progress(value);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;

    bool lock_acquired;
    int lock_error;
    int unlock_error;
    bool is_complete_flag;

    this->abort_if_not_initialized("ft_achievement::is_goal_complete");
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (false);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (false);
        }
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    is_complete_flag = (entry->value.get_progress() >= entry->value.get_target());
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (false);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (is_complete_flag);
}

bool ft_achievement::is_complete() const noexcept
{
    const Pair<int, ft_goal> *goal_entry;
    const Pair<int, ft_goal> *goals_end;
    size_t                    goals_count;
    size_t                    index;
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_achievement::is_complete");
    goals_count = this->_goals.size();
    goals_end = this->_goals.end();
    if (goals_count == 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (true);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (false);
    }
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (goal_entry->value.get_progress() < goal_entry->value.get_target())
        {
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                this->set_error(unlock_error);
                return (false);
            }
            this->set_error(FT_ERR_SUCCESS);
            return (false);
        }
        goal_entry++;
        index += 1;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (false);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (true);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_achievement::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
