#include "game_achievement.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

ft_goal::ft_goal() noexcept
    : _target(0), _progress(0), _mutex(ft_nullptr),
      _initialized_state(ft_goal::_state_uninitialized)
{
    return ;
}

ft_goal::~ft_goal() noexcept
{
    if (this->_initialized_state == ft_goal::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_goal::~ft_goal",
            "destructor called while object is uninitialized");
        return ;
    }
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
        return (FT_ERR_INVALID_STATE);
    }
    this->_target = 0;
    this->_progress = 0;
    this->_initialized_state = ft_goal::_state_initialized;
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
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_target = other._target;
    this->_progress = other._progress;
    return (FT_ERR_SUCCESS);
}

int ft_goal::initialize(ft_goal &&other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_goal::_state_initialized)
    {
        other.abort_lifecycle_error("ft_goal::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_target = other._target;
    this->_progress = other._progress;
    return (FT_ERR_SUCCESS);
}

int ft_goal::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_goal::_state_initialized)
    {
        this->abort_lifecycle_error("ft_goal::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_target = 0;
    this->_progress = 0;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_goal::_state_destroyed;
    return (disable_error);
}

int ft_goal::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_goal::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
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

int ft_goal::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_goal::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_goal::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_goal::lock_internal(bool *lock_acquired) const noexcept
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

int ft_goal::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_goal::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_goal::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_goal::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_goal::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_goal::get_target() const noexcept
{
    this->abort_if_not_initialized("ft_goal::get_target");
    return (this->_target);
}

void ft_goal::set_target(int target) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_goal::set_target");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_target = target;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_goal::get_progress() const noexcept
{
    this->abort_if_not_initialized("ft_goal::get_progress");
    return (this->_progress);
}

void ft_goal::set_progress(int value) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_goal::set_progress");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_progress = value;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_goal::add_progress(int delta) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_goal::add_progress");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_progress += delta;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_goal::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_goal::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _mutex(ft_nullptr),
      _initialized_state(ft_achievement::_state_uninitialized)
{
    return ;
}

ft_achievement::~ft_achievement() noexcept
{
    if (this->_initialized_state == ft_achievement::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_achievement::~ft_achievement",
            "destructor called while object is uninitialized");
        return ;
    }
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
    if (this->_initialized_state == ft_achievement::_state_initialized)
    {
        this->abort_lifecycle_error("ft_achievement::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_goals.clear();
    this->_initialized_state = ft_achievement::_state_initialized;
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
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
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
            return (FT_ERR_INVALID_STATE);
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
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
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
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
            return (FT_ERR_INVALID_STATE);
        }
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int ft_achievement::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_achievement::_state_initialized)
    {
        this->abort_lifecycle_error("ft_achievement::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_goals.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_achievement::_state_destroyed;
    return (disable_error);
}

int ft_achievement::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_achievement::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
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

int ft_achievement::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_achievement::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_achievement::lock_internal(bool *lock_acquired) const noexcept
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

int ft_achievement::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_achievement::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_achievement::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_achievement::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_achievement::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_achievement::get_id() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::get_id");
    return (this->_id);
}

void ft_achievement::set_id(int id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_achievement::set_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_id = id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    this->abort_if_not_initialized("ft_achievement::get_goals");
    return (this->_goals);
}

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::get_goals const");
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

    this->abort_if_not_initialized("ft_achievement::set_goals");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_goals.clear();
    goals_count = goals.size();
    goals_end = goals.end();
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
            break ;
        new_goal.set_target(goal_entry->value.get_target());
        new_goal.set_progress(goal_entry->value.get_progress());
        this->_goals.insert(goal_entry->key, new_goal);
        (void)new_goal.destroy();
        goal_entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;

    this->abort_if_not_initialized("ft_achievement::get_goal");
    if (id < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
        return (FT_ERR_NOT_FOUND);
    return (entry->value.get_target());
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;

    this->abort_if_not_initialized("ft_achievement::set_goal");
    if (id < 0)
        return ;
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
            return ;
        new_goal.set_target(goal);
        new_goal.set_progress(0);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        return ;
    }
    entry->value.set_target(goal);
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;

    this->abort_if_not_initialized("ft_achievement::get_progress");
    if (id < 0)
        return (0);
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
        return (0);
    return (entry->value.get_progress());
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;

    this->abort_if_not_initialized("ft_achievement::set_progress");
    if (id < 0)
        return ;
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
            return ;
        new_goal.set_target(0);
        new_goal.set_progress(progress);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        return ;
    }
    entry->value.set_progress(progress);
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    Pair<int, ft_goal> *entry;
    ft_goal             new_goal;

    this->abort_if_not_initialized("ft_achievement::add_progress");
    if (id < 0)
        return ;
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        if (new_goal.initialize() != FT_ERR_SUCCESS)
            return ;
        new_goal.set_target(0);
        new_goal.set_progress(value);
        this->_goals.insert(id, new_goal);
        (void)new_goal.destroy();
        return ;
    }
    entry->value.add_progress(value);
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;

    this->abort_if_not_initialized("ft_achievement::is_goal_complete");
    if (id < 0)
        return (false);
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
        return (false);
    if (entry->value.get_progress() < entry->value.get_target())
        return (false);
    return (true);
}

bool ft_achievement::is_complete() const noexcept
{
    const Pair<int, ft_goal> *goal_entry;
    const Pair<int, ft_goal> *goals_end;
    size_t                    goals_count;
    size_t                    index;

    this->abort_if_not_initialized("ft_achievement::is_complete");
    goals_count = this->_goals.size();
    goals_end = this->_goals.end();
    if (goals_count == 0)
        return (true);
    goal_entry = goals_end - goals_count;
    index = 0;
    while (index < goals_count)
    {
        if (goal_entry->value.get_progress() < goal_entry->value.get_target())
            return (false);
        goal_entry++;
        index += 1;
    }
    return (true);
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_achievement::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_achievement::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
