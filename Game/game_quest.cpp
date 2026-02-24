#include "game_quest.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0), _description(), _objective(),
      _reward_experience(0), _reward_items(), _mutex(ft_nullptr),
      _initialized_state(ft_quest::_state_uninitialized)
{
    return ;
}

ft_quest::~ft_quest() noexcept
{
    if (this->_initialized_state != ft_quest::_state_initialized)
    {
        this->_initialized_state = ft_quest::_state_destroyed;
        return ;
    }
    (void)this->destroy();
    return ;
}

void ft_quest::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_quest lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_quest::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_quest::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_quest::initialize() noexcept
{
    int description_error;
    int objective_error;
    int reward_items_error;

    if (this->_initialized_state == ft_quest::_state_initialized)
    {
        this->abort_lifecycle_error("ft_quest::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_phases = 0;
    this->_current_phase = 0;
    description_error = this->_description.initialize();
    if (description_error != FT_ERR_SUCCESS)
        return (description_error);
    objective_error = this->_objective.initialize();
    if (objective_error != FT_ERR_SUCCESS)
    {
        (void)this->_description.destroy();
        return (objective_error);
    }
    reward_items_error = this->_reward_items.initialize();
    if (reward_items_error != FT_ERR_SUCCESS)
    {
        (void)this->_description.destroy();
        (void)this->_objective.destroy();
        return (reward_items_error);
    }
    this->_description.clear();
    this->_objective.clear();
    this->_reward_experience = 0;
    this->_reward_items.clear();
    this->_initialized_state = ft_quest::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_quest::initialize(const ft_quest &other) noexcept
{
    int initialize_error;
    int copy_error;

    if (other._initialized_state != ft_quest::_state_initialized)
    {
        other.abort_lifecycle_error("ft_quest::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = other._description;
    this->_objective = other._objective;
    this->_reward_experience = other._reward_experience;
    copy_error = this->_reward_items.copy_from(other._reward_items);
    if (copy_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_quest::_state_destroyed;
        return (copy_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_quest::initialize(ft_quest &&other) noexcept
{
    int initialize_error;
    int copy_error;

    if (other._initialized_state != ft_quest::_state_initialized)
    {
        other.abort_lifecycle_error("ft_quest::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = other._description;
    this->_objective = other._objective;
    this->_reward_experience = other._reward_experience;
    copy_error = this->_reward_items.copy_from(other._reward_items);
    if (copy_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_quest::_state_destroyed;
        return (copy_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_quest::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_quest::_state_initialized)
    {
        this->_initialized_state = ft_quest::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    this->_reward_items.clear();
    this->_description.clear();
    this->_objective.clear();
    this->_id = 0;
    this->_phases = 0;
    this->_current_phase = 0;
    this->_reward_experience = 0;
    (void)this->_description.destroy();
    (void)this->_objective.destroy();
    (void)this->_reward_items.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_quest::_state_destroyed;
    return (disable_error);
}

int ft_quest::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_quest::enable_thread_safety");
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

int ft_quest::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_quest::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_quest::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_quest::lock_internal(bool *lock_acquired) const noexcept
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

int ft_quest::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_quest::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_quest::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_quest::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_quest::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_quest::get_id() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_id");
    return (this->_id);
}

void ft_quest::set_id(int id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (id >= 0)
        this->_id = id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_quest::get_phases() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_phases");
    return (this->_phases);
}

void ft_quest::set_phases(int phases) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_phases");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (phases >= 0)
    {
        this->_phases = phases;
        if (this->_current_phase > this->_phases)
            this->_current_phase = this->_phases;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_quest::get_current_phase() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_current_phase");
    return (this->_current_phase);
}

void ft_quest::set_current_phase(int phase) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_current_phase");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (phase >= 0 && phase <= this->_phases)
        this->_current_phase = phase;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_quest::get_description() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_description");
    return (this->_description);
}

void ft_quest::set_description(const ft_string &description) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_description");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_description = description;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_quest::get_objective() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_objective");
    return (this->_objective);
}

void ft_quest::set_objective(const ft_string &objective) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_objective");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_objective = objective;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_quest::get_reward_experience() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_reward_experience");
    return (this->_reward_experience);
}

void ft_quest::set_reward_experience(int experience) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_reward_experience");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (experience >= 0)
        this->_reward_experience = experience;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() noexcept
{
    this->abort_if_not_initialized("ft_quest::get_reward_items");
    return (this->_reward_items);
}

const ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_reward_items const");
    return (this->_reward_items);
}

void ft_quest::set_reward_items(const ft_vector<ft_sharedptr<ft_item> > &items) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::set_reward_items");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    (void)this->_reward_items.copy_from(items);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

bool ft_quest::is_complete() const noexcept
{
    this->abort_if_not_initialized("ft_quest::is_complete");
    return (this->_current_phase >= this->_phases);
}

void ft_quest::advance_phase() noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_quest::advance_phase");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (this->_current_phase < this->_phases)
        this->_current_phase += 1;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_quest::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_quest::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
