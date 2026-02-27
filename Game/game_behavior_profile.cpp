#include "../PThread/pthread_internal.hpp"
#include "game_behavior_profile.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_behavior_profile::_last_error = FT_ERR_SUCCESS;
static void game_behavior_copy_action_vector(
    const ft_vector<ft_behavior_action> &source,
    ft_vector<ft_behavior_action> &destination)
{
    const ft_behavior_action *entry;
    const ft_behavior_action *entry_end;

    destination.clear();
    entry = source.begin();
    entry_end = source.end();
    while (entry != entry_end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

ft_behavior_profile::ft_behavior_profile() noexcept
    : _profile_id(0), _aggression_weight(0.0), _caution_weight(0.0),
      _actions(), _mutex(ft_nullptr),
      _initialized_state(ft_behavior_profile::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_profile::~ft_behavior_profile() noexcept
{
    if (this->_initialized_state == ft_behavior_profile::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_behavior_profile::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_behavior_profile::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_behavior_profile lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_behavior_profile::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_behavior_profile::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_behavior_profile::initialize() noexcept
{
    if (this->_initialized_state == ft_behavior_profile::_state_initialized)
    {
        this->abort_lifecycle_error("ft_behavior_profile::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_profile_id = 0;
    this->_aggression_weight = 0.0;
    this->_caution_weight = 0.0;
    this->_actions.clear();
    this->_initialized_state = ft_behavior_profile::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_profile::initialize(const ft_behavior_profile &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_behavior_profile::_state_initialized)
    {
        other.abort_lifecycle_error("ft_behavior_profile::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_behavior_profile::_state_initialized)
        (void)this->destroy();
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_profile_id = other._profile_id;
    this->_aggression_weight = other._aggression_weight;
    this->_caution_weight = other._caution_weight;
    game_behavior_copy_action_vector(other._actions, this->_actions);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_profile::initialize(ft_behavior_profile &&other) noexcept
{
    return (this->initialize(static_cast<const ft_behavior_profile &>(other)));
}

int ft_behavior_profile::initialize(int profile_id, double aggression_weight,
    double caution_weight,
    const ft_vector<ft_behavior_action> &actions) noexcept
{
    int initialize_error;

    if (this->_initialized_state == ft_behavior_profile::_state_initialized)
        (void)this->destroy();
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_profile_id = profile_id;
    this->_aggression_weight = aggression_weight;
    this->_caution_weight = caution_weight;
    game_behavior_copy_action_vector(actions, this->_actions);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_profile::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_behavior_profile::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_profile_id = 0;
    this->_aggression_weight = 0.0;
    this->_caution_weight = 0.0;
    this->_actions.clear();
    this->_initialized_state = ft_behavior_profile::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_behavior_profile::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_behavior_profile::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
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

int ft_behavior_profile::disable_thread_safety() noexcept
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

bool ft_behavior_profile::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::is_thread_safe");
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_behavior_profile::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_behavior_profile::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_behavior_profile::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::lock");
    int lock_error = this->lock_internal(lock_acquired);
    this->set_error(lock_error);
    return (lock_error);
}

void ft_behavior_profile::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::unlock");
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    this->set_error(unlock_error);
    return ;
}

int ft_behavior_profile::get_profile_id() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_profile_id");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_profile_id);
}

void ft_behavior_profile::set_profile_id(int profile_id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_behavior_profile::set_profile_id");
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_profile_id = profile_id;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

double ft_behavior_profile::get_aggression_weight() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_aggression_weight");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_aggression_weight);
}

void ft_behavior_profile::set_aggression_weight(double aggression_weight) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_behavior_profile::set_aggression_weight");
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_aggression_weight = aggression_weight;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

double ft_behavior_profile::get_caution_weight() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_caution_weight");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_caution_weight);
}

void ft_behavior_profile::set_caution_weight(double caution_weight) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_behavior_profile::set_caution_weight");
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_caution_weight = caution_weight;
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_behavior_action> &ft_behavior_profile::get_actions() noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_actions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_actions);
}

const ft_vector<ft_behavior_action> &ft_behavior_profile::get_actions() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_actions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_actions);
}

void ft_behavior_profile::set_actions(
    const ft_vector<ft_behavior_action> &actions) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_behavior_profile::set_actions");
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_behavior_copy_action_vector(actions, this->_actions);
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_behavior_profile::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_profile::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

int ft_behavior_profile::get_error() const noexcept
{
    return (ft_behavior_profile::_last_error);
}

const char *ft_behavior_profile::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_behavior_profile::set_error(int error_code) const noexcept
{
    ft_behavior_profile::_last_error = error_code;
    return ;
}
