#include "../PThread/pthread_internal.hpp"
#include "game_quest.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Template/vector.hpp"

thread_local int32_t game_quest::_last_error = FT_ERR_SUCCESS;

int32_t game_quest::set_error(int32_t error_code) noexcept
{
    game_quest::_last_error = error_code;
    return (error_code);
}

int32_t    game_quest::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_quest::get_error");
    return (game_quest::_last_error);
}

const char    *game_quest::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_quest::get_error_str");
    return (ft_strerror(game_quest::_last_error));
}

game_quest::game_quest() noexcept
    : _id(0), _phases(0), _current_phase(0), _description(), _objective(),
      _reward_experience(0), _reward_items(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_quest::~game_quest() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    (void)this->destroy();
    return ;
}

int32_t game_quest::initialize() noexcept
{
    int32_t description_error;
    int32_t objective_error;
    int32_t reward_items_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_quest::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_phases = 0;
    this->_current_phase = 0;
    description_error = this->_description.initialize();
    if (description_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(description_error);
        return (description_error);
    }
    objective_error = this->_objective.initialize();
    if (objective_error != FT_ERR_SUCCESS)
    {
        (void)this->_description.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(objective_error);
        return (objective_error);
    }
    reward_items_error = this->_reward_items.initialize();
    if (reward_items_error != FT_ERR_SUCCESS)
    {
        (void)this->_description.destroy();
        (void)this->_objective.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(reward_items_error);
        return (reward_items_error);
    }
    this->_reward_experience = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_quest::initialize(const game_quest &other) noexcept
{
    int32_t initialize_error;
    int32_t copy_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_quest::initialize(copy)", "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->set_error(initialize_error);
            return (initialize_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = other._description;
    this->_objective = other._objective;
    this->_reward_experience = other._reward_experience;
    copy_error = this->_reward_items.copy_from(other._reward_items);
    if (copy_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(copy_error);
        return (copy_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_quest::initialize(game_quest &&other) noexcept
{
    return (this->move(other));
}

int32_t game_quest::move(game_quest &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_quest::move", "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize(static_cast<const game_quest &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t game_quest::destroy() noexcept
{
    int32_t description_error;
    int32_t objective_error;
    int32_t reward_items_error;
    int32_t disable_error;
    int32_t first_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    first_error = FT_ERR_SUCCESS;
    description_error = this->_description.destroy();
    if (first_error == FT_ERR_SUCCESS && description_error != FT_ERR_SUCCESS)
        first_error = description_error;
    objective_error = this->_objective.destroy();
    if (first_error == FT_ERR_SUCCESS && objective_error != FT_ERR_SUCCESS)
        first_error = objective_error;
    reward_items_error = this->_reward_items.destroy();
    if (first_error == FT_ERR_SUCCESS && reward_items_error != FT_ERR_SUCCESS)
        first_error = reward_items_error;
    this->_id = 0;
    this->_phases = 0;
    this->_current_phase = 0;
    this->_reward_experience = 0;
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

int32_t game_quest::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::enable_thread_safety");
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

int32_t game_quest::disable_thread_safety() noexcept
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

ft_bool game_quest::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_quest::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t game_quest::unlock_internal(ft_bool lock_acquired) const noexcept
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

int32_t game_quest::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::lock");
    return (this->lock_internal(lock_acquired));
}

void game_quest::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_quest::get_id() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    int32_t identifier = this->_id;
    (void)this->unlock_internal(lock_acquired);
    return (identifier);
}

void game_quest::set_id(int32_t id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (id >= 0);
    if (valid)
        this->_id = id;
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_quest::get_phases() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_phases");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    int32_t phases_value = this->_phases;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (phases_value);
}

void game_quest::set_phases(int32_t phases) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_phases");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (phases >= 0);
    if (valid)
    {
        this->_phases = phases;
        if (this->_current_phase > this->_phases)
            this->_current_phase = this->_phases;
    }
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_quest::get_current_phase() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_current_phase");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    int32_t phase_value = this->_current_phase;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (phase_value);
}

void game_quest::set_current_phase(int32_t phase) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_current_phase");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (phase >= 0 && phase <= this->_phases);
    if (valid)
        this->_current_phase = phase;
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &game_quest::get_description() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_description");
    return (this->_description);
}

void game_quest::set_description(const ft_string &description) noexcept
{
    ft_bool lock_acquired;

    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_description");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_description = description;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &game_quest::get_objective() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_objective");
    return (this->_objective);
}

void game_quest::set_objective(const ft_string &objective) noexcept
{
    ft_bool lock_acquired;

    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_objective");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_objective = objective;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_quest::get_reward_experience() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_reward_experience");
    return (this->_reward_experience);
}

void game_quest::set_reward_experience(int32_t experience) noexcept
{
    ft_bool lock_acquired;

    int32_t lock_error;
    ft_bool valid;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_reward_experience");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (experience >= 0);
    if (valid)
        this->_reward_experience = experience;
    if (valid == FT_TRUE)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_sharedptr<game_item> > &game_quest::get_reward_items() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_reward_items");
    return (this->_reward_items);
}

const ft_vector<ft_sharedptr<game_item> > &game_quest::get_reward_items() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::get_reward_items const");
    return (this->_reward_items);
}

void game_quest::set_reward_items(const ft_vector<ft_sharedptr<game_item> > &items) noexcept
{
    ft_bool lock_acquired;

    int32_t lock_error;
    int32_t copy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::set_reward_items");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    copy_error = this->_reward_items.copy_from(items);
    (void)this->unlock_internal(lock_acquired);
    if (copy_error != FT_ERR_SUCCESS)
    {
        this->set_error(copy_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_bool game_quest::is_complete() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::is_complete");
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool result;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (FT_FALSE);
    }
    result = (this->_current_phase >= this->_phases);
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

void game_quest::advance_phase() noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_quest::advance_phase");
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_current_phase < this->_phases)
        this->_current_phase += 1;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}
