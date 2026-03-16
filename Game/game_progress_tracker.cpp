#include "../PThread/pthread_internal.hpp"
#include "game_progress_tracker.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local uint32_t game_progress_tracker::_last_error = FT_ERR_SUCCESS;

uint32_t game_progress_tracker::set_error(uint32_t error_code) noexcept
{
    game_progress_tracker::_last_error = error_code;
    return (error_code);
}

int32_t game_progress_tracker::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_progress_tracker::get_error");
    return (static_cast<int32_t>(game_progress_tracker::_last_error));
}

const char *game_progress_tracker::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_progress_tracker::get_error_str");
    return (ft_strerror(game_progress_tracker::_last_error));
}

game_progress_tracker::game_progress_tracker() noexcept
    : _achievements(), _quests(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_progress_tracker::game_progress_tracker(const game_progress_tracker &other) noexcept
    : _achievements(), _quests(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_progress_tracker::game_progress_tracker(copy)",
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
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
    }
    return ;
}

game_progress_tracker::game_progress_tracker(game_progress_tracker &&other) noexcept
    : _achievements(), _quests(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_progress_tracker::game_progress_tracker(move)",
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
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

game_progress_tracker::~game_progress_tracker() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_progress_tracker::initialize() noexcept
{
    int32_t error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_progress_tracker::initialize",
            "called while object is already initialised");
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
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_progress_tracker::initialize(const game_progress_tracker &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_achievement> *achievement_entry;
    const Pair<int32_t, game_achievement> *achievement_end;
    const Pair<int32_t, game_quest> *quest_entry;
    const Pair<int32_t, game_quest> *quest_end;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_progress_tracker::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(static_cast<uint32_t>(other.get_error()));
        return (FT_ERR_SUCCESS);
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

int32_t game_progress_tracker::initialize(game_progress_tracker &&other) noexcept
{
    return (this->move(other));
}

int32_t game_progress_tracker::move(game_progress_tracker &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_progress_tracker::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize(static_cast<const game_progress_tracker &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    if (other._initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t game_progress_tracker::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_achievements.clear();
    this->_quests.clear();
    (void)this->_achievements.destroy();
    (void)this->_quests.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_progress_tracker::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::enable_thread_safety");
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

int32_t game_progress_tracker::disable_thread_safety() noexcept
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

ft_bool game_progress_tracker::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_progress_tracker::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

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

int32_t game_progress_tracker::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }

    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_progress_tracker::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::lock");
    return (this->lock_internal(lock_acquired));
}

void game_progress_tracker::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int32_t, game_achievement> &game_progress_tracker::get_achievements() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::get_achievements");
    return (this->_achievements);
}

const ft_map<int32_t, game_achievement> &game_progress_tracker::get_achievements() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::get_achievements const");
    return (this->_achievements);
}

ft_map<int32_t, game_quest> &game_progress_tracker::get_quests() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::get_quests");
    return (this->_quests);
}

const ft_map<int32_t, game_quest> &game_progress_tracker::get_quests() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::get_quests const");
    return (this->_quests);
}

void game_progress_tracker::set_achievements(
    const ft_map<int32_t, game_achievement> &achievements) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_achievement> *entry;
    const Pair<int32_t, game_achievement> *entry_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::set_achievements");
    int32_t lock_error;

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

void game_progress_tracker::set_quests(const ft_map<int32_t, game_quest> &quests) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_quest> *entry;
    const Pair<int32_t, game_quest> *entry_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::set_quests");
    int32_t lock_error;

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

int32_t game_progress_tracker::register_achievement(
    const game_achievement &achievement) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::register_achievement");
    int32_t lock_error;

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

int32_t game_progress_tracker::register_quest(const game_quest &quest) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::register_quest");
    int32_t lock_error;

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

int32_t game_progress_tracker::update_goal_target(int32_t achievement_id, int32_t goal_id,
    int32_t target) noexcept
{
    Pair<int32_t, game_achievement> *achievement_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::update_goal_target");
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

int32_t game_progress_tracker::update_goal_progress(int32_t achievement_id, int32_t goal_id,
    int32_t progress) noexcept
{
    Pair<int32_t, game_achievement> *achievement_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::update_goal_progress");
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

int32_t game_progress_tracker::add_goal_progress(int32_t achievement_id, int32_t goal_id,
    int32_t value) noexcept
{
    Pair<int32_t, game_achievement> *achievement_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::add_goal_progress");
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

ft_bool game_progress_tracker::is_achievement_complete(int32_t achievement_id) const noexcept
{
    const Pair<int32_t, game_achievement> *achievement_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::is_achievement_complete");
    achievement_entry = this->_achievements.find(achievement_id);
    if (achievement_entry == this->_achievements.end())
        return (FT_FALSE);
    return (achievement_entry->value.is_complete());
}

int32_t game_progress_tracker::set_quest_phase(int32_t quest_id, int32_t phase) noexcept
{
    Pair<int32_t, game_quest> *quest_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::set_quest_phase");
    quest_entry = this->_quests.find(quest_id);
    if (quest_entry == this->_quests.end())
        return (FT_ERR_NOT_FOUND);
    quest_entry->value.set_current_phase(phase);
    return (FT_ERR_SUCCESS);
}

int32_t game_progress_tracker::advance_quest_phase(int32_t quest_id) noexcept
{
    Pair<int32_t, game_quest> *quest_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::advance_quest_phase");
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

ft_bool game_progress_tracker::is_quest_complete(int32_t quest_id) const noexcept
{
    const Pair<int32_t, game_quest> *quest_entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_progress_tracker::is_quest_complete");
    quest_entry = this->_quests.find(quest_id);
    if (quest_entry == this->_quests.end())
        return (FT_FALSE);
    return (quest_entry->value.is_complete());
}
