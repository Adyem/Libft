#include "../PThread/pthread_internal.hpp"
#include "game_progress_tracker.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_progress_tracker::_last_error = FT_ERR_SUCCESS;

void ft_progress_tracker::set_error(int error_code) const noexcept
{
    ft_progress_tracker::_last_error = error_code;
    return ;
}

int ft_progress_tracker::get_error() const noexcept
{
    return (ft_progress_tracker::_last_error);
}

const char *ft_progress_tracker::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_progress_tracker::ft_progress_tracker() noexcept
    : _achievements(), _quests(), _mutex(ft_nullptr),
      _initialized_state(ft_progress_tracker::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_progress_tracker::~ft_progress_tracker() noexcept
{
    if (this->_initialized_state == ft_progress_tracker::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_progress_tracker::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_progress_tracker::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_progress_tracker lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_progress_tracker::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_progress_tracker::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_progress_tracker::initialize() noexcept
{
    int error;

    if (this->_initialized_state == ft_progress_tracker::_state_initialized)
    {
        this->abort_lifecycle_error("ft_progress_tracker::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    error = this->_achievements.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        this->set_error(error);
        return (error);
    }
    error = this->_quests.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)this->_achievements.destroy();
        this->set_error(error);
        return (error);
    }
    this->_achievements.clear();
    this->_quests.clear();
    this->_initialized_state = ft_progress_tracker::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::initialize(const ft_progress_tracker &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<int, ft_achievement> *achievement_entry;
    const Pair<int, ft_achievement> *achievement_end;
    const Pair<int, ft_quest> *quest_entry;
    const Pair<int, ft_quest> *quest_end;

    if (other._initialized_state != ft_progress_tracker::_state_initialized)
    {
        other.abort_lifecycle_error("ft_progress_tracker::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    count = other._achievements.size();
    achievement_end = other._achievements.end();
    achievement_entry = achievement_end - count;
    index = 0;
    while (index < count)
    {
        this->_achievements.insert(achievement_entry->key, achievement_entry->value);
        achievement_entry++;
        index += 1;
    }
    count = other._quests.size();
    quest_end = other._quests.end();
    quest_entry = quest_end - count;
    index = 0;
    while (index < count)
    {
        this->_quests.insert(quest_entry->key, quest_entry->value);
        quest_entry++;
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::initialize(ft_progress_tracker &&other) noexcept
{
    return (this->initialize(static_cast<const ft_progress_tracker &>(other)));
}

int ft_progress_tracker::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_progress_tracker::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_achievements.clear();
    this->_quests.clear();
    (void)this->_achievements.destroy();
    (void)this->_quests.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_progress_tracker::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_progress_tracker::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_progress_tracker::enable_thread_safety");
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

int ft_progress_tracker::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int destroy_error;

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

bool ft_progress_tracker::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_progress_tracker::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
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

int ft_progress_tracker::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int unlock_error;

    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    return (unlock_error);
}

int ft_progress_tracker::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_progress_tracker::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int, ft_achievement> &ft_progress_tracker::get_achievements() noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::get_achievements");
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_progress_tracker::get_achievements() const noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::get_achievements const");
    return (this->_achievements);
}

ft_map<int, ft_quest> &ft_progress_tracker::get_quests() noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::get_quests");
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_progress_tracker::get_quests() const noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::get_quests const");
    return (this->_quests);
}

void ft_progress_tracker::set_achievements(
    const ft_map<int, ft_achievement> &achievements) noexcept
{
    bool lock_acquired;
    size_t count;
    size_t index;
    const Pair<int, ft_achievement> *entry;
    const Pair<int, ft_achievement> *entry_end;

    this->abort_if_not_initialized("ft_progress_tracker::set_achievements");
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_achievements.clear();
    count = achievements.size();
    entry_end = achievements.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_achievements.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_progress_tracker::set_quests(const ft_map<int, ft_quest> &quests) noexcept
{
    bool lock_acquired;
    size_t count;
    size_t index;
    const Pair<int, ft_quest> *entry;
    const Pair<int, ft_quest> *entry_end;

    this->abort_if_not_initialized("ft_progress_tracker::set_quests");
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_quests.clear();
    count = quests.size();
    entry_end = quests.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_quests.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_progress_tracker::register_achievement(
    const ft_achievement &achievement) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_progress_tracker::register_achievement");
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_achievements.insert(achievement.get_id(), achievement);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::register_quest(const ft_quest &quest) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_progress_tracker::register_quest");
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_quests.insert(quest.get_id(), quest);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::update_goal_target(int achievement_id, int goal_id,
    int target) noexcept
{
    Pair<int, ft_achievement> *achievement_entry;

    this->abort_if_not_initialized("ft_progress_tracker::update_goal_target");
    achievement_entry = this->_achievements.find(achievement_id);
    if (achievement_entry == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_entry->value.set_goal(goal_id, target);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::update_goal_progress(int achievement_id, int goal_id,
    int progress) noexcept
{
    Pair<int, ft_achievement> *achievement_entry;

    this->abort_if_not_initialized("ft_progress_tracker::update_goal_progress");
    achievement_entry = this->_achievements.find(achievement_id);
    if (achievement_entry == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_entry->value.set_progress(goal_id, progress);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::add_goal_progress(int achievement_id, int goal_id,
    int value) noexcept
{
    Pair<int, ft_achievement> *achievement_entry;

    this->abort_if_not_initialized("ft_progress_tracker::add_goal_progress");
    achievement_entry = this->_achievements.find(achievement_id);
    if (achievement_entry == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_entry->value.add_progress(goal_id, value);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

bool ft_progress_tracker::is_achievement_complete(int achievement_id) const noexcept
{
    const Pair<int, ft_achievement> *achievement_entry;

    this->abort_if_not_initialized("ft_progress_tracker::is_achievement_complete");
    achievement_entry = this->_achievements.find(achievement_id);
    if (achievement_entry == this->_achievements.end())
        return (false);
    return (achievement_entry->value.is_complete());
}

int ft_progress_tracker::set_quest_phase(int quest_id, int phase) noexcept
{
    Pair<int, ft_quest> *quest_entry;

    this->abort_if_not_initialized("ft_progress_tracker::set_quest_phase");
    quest_entry = this->_quests.find(quest_id);
    if (quest_entry == this->_quests.end())
        return (FT_ERR_NOT_FOUND);
    quest_entry->value.set_current_phase(phase);
    return (FT_ERR_SUCCESS);
}

int ft_progress_tracker::advance_quest_phase(int quest_id) noexcept
{
    Pair<int, ft_quest> *quest_entry;

    this->abort_if_not_initialized("ft_progress_tracker::advance_quest_phase");
    quest_entry = this->_quests.find(quest_id);
    if (quest_entry == this->_quests.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    if (quest_entry->value.is_complete())
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    quest_entry->value.advance_phase();
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

bool ft_progress_tracker::is_quest_complete(int quest_id) const noexcept
{
    const Pair<int, ft_quest> *quest_entry;

    this->abort_if_not_initialized("ft_progress_tracker::is_quest_complete");
    quest_entry = this->_quests.find(quest_id);
    if (quest_entry == this->_quests.end())
        return (false);
    return (quest_entry->value.is_complete());
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_progress_tracker::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_progress_tracker::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
