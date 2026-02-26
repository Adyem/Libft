#include "game_behavior_table.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_behavior_table::_last_error = FT_ERR_SUCCESS;

ft_behavior_table::ft_behavior_table() noexcept
    : _profiles(), _mutex(ft_nullptr),
      _initialized_state(ft_behavior_table::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_table::~ft_behavior_table() noexcept
{
    if (this->_initialized_state == ft_behavior_table::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_behavior_table::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_behavior_table::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_behavior_table lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_behavior_table::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_behavior_table::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_behavior_table::initialize() noexcept
{
    if (this->_initialized_state == ft_behavior_table::_state_initialized)
    {
        this->abort_lifecycle_error("ft_behavior_table::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    int initialize_error = this->_profiles.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_initialized_state = ft_behavior_table::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_table::initialize(const ft_behavior_table &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<int, ft_behavior_profile> *entry;
    const Pair<int, ft_behavior_profile> *entry_end;

    if (other._initialized_state != ft_behavior_table::_state_initialized)
    {
        other.abort_lifecycle_error("ft_behavior_table::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    count = other._profiles.size();
    entry_end = other._profiles.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_profiles.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_table::initialize(ft_behavior_table &&other) noexcept
{
    return (this->initialize(static_cast<const ft_behavior_table &>(other)));
}

int ft_behavior_table::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_behavior_table::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_profiles.clear();
    (void)this->_profiles.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_behavior_table::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_behavior_table::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_behavior_table::enable_thread_safety");
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

int ft_behavior_table::disable_thread_safety() noexcept
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

bool ft_behavior_table::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::is_thread_safe");
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_behavior_table::lock_internal(bool *lock_acquired) const noexcept
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

int ft_behavior_table::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_behavior_table::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::lock");
    int lock_error = this->lock_internal(lock_acquired);
    this->set_error(lock_error);
    return (lock_error);
}

void ft_behavior_table::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::unlock");
    int unlock_error = this->unlock_internal(lock_acquired);
    this->set_error(unlock_error);
    return ;
}

ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::get_profiles");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_profiles);
}

const ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::get_profiles const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_profiles);
}

void ft_behavior_table::set_profiles(
    const ft_map<int, ft_behavior_profile> &profiles) noexcept
{
    bool lock_acquired;
    size_t count;
    size_t index;
    const Pair<int, ft_behavior_profile> *entry;
    const Pair<int, ft_behavior_profile> *entry_end;

    this->abort_if_not_initialized("ft_behavior_table::set_profiles");
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_profiles.clear();
    count = profiles.size();
    entry_end = profiles.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_profiles.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_behavior_table::register_profile(const ft_behavior_profile &profile) noexcept
{
    bool lock_acquired;
    int profile_identifier;

    this->abort_if_not_initialized("ft_behavior_table::register_profile");
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    profile_identifier = profile.get_profile_id();
    this->_profiles.insert(profile_identifier, profile);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_table::fetch_profile(int profile_id,
    ft_behavior_profile &profile) const noexcept
{
    const Pair<int, ft_behavior_profile> *entry;

    this->abort_if_not_initialized("ft_behavior_table::fetch_profile");
    entry = this->_profiles.find(profile_id);
    if (entry == this->_profiles.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    int initialize_error = profile.initialize(entry->value);
    this->set_error(initialize_error);
    return (initialize_error);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_behavior_table::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_behavior_table::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

int ft_behavior_table::get_error() const noexcept
{
    return (ft_behavior_table::_last_error);
}

const char *ft_behavior_table::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_behavior_table::set_error(int error_code) const noexcept
{
    ft_behavior_table::_last_error = error_code;
    return ;
}
