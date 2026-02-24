#include "ft_region_definition.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_region_definition::ft_region_definition() noexcept
    : _region_id(0), _name(), _description(), _recommended_level(0),
      _mutex(ft_nullptr),
      _initialized_state(ft_region_definition::_state_uninitialized)
{
    return ;
}

ft_region_definition::~ft_region_definition() noexcept
{
    if (this->_initialized_state != ft_region_definition::_state_initialized)
    {
        this->_initialized_state = ft_region_definition::_state_destroyed;
        return ;
    }
    (void)this->destroy();
    return ;
}

void ft_region_definition::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_region_definition lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_region_definition::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_region_definition::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_region_definition::initialize() noexcept
{
    if (this->_initialized_state == ft_region_definition::_state_initialized)
    {
        this->abort_lifecycle_error("ft_region_definition::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_region_definition::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_region_definition::initialize(const ft_region_definition &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_region_definition::_state_initialized)
    {
        other.abort_lifecycle_error("ft_region_definition::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    return (FT_ERR_SUCCESS);
}

int ft_region_definition::initialize(ft_region_definition &&other) noexcept
{
    return (this->initialize(static_cast<const ft_region_definition &>(other)));
}

int ft_region_definition::initialize(int region_id, const ft_string &name,
    const ft_string &description, int recommended_level) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_region_id = region_id;
    this->_name = name;
    this->_description = description;
    this->_recommended_level = recommended_level;
    return (FT_ERR_SUCCESS);
}

int ft_region_definition::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_region_definition::_state_initialized)
    {
        this->_initialized_state = ft_region_definition::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_region_definition::_state_destroyed;
    return (disable_error);
}

int ft_region_definition::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_region_definition::enable_thread_safety");
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

int ft_region_definition::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_region_definition::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_region_definition::lock_internal(bool *lock_acquired) const noexcept
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

int ft_region_definition::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_region_definition::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_region_definition::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_region_definition::get_region_id() const noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::get_region_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (0);
    (void)this->unlock_internal(lock_acquired);
    return (this->_region_id);
}

void ft_region_definition::set_region_id(int region_id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_region_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_region_id = region_id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_region_definition::get_name() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_name");
    return (this->_name);
}

void ft_region_definition::set_name(const ft_string &name) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_name");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_name = name;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_region_definition::get_description() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_description");
    return (this->_description);
}

void ft_region_definition::set_description(const ft_string &description) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_description");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_description = description;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_region_definition::get_recommended_level() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_recommended_level");
    return (this->_recommended_level);
}

void ft_region_definition::set_recommended_level(int recommended_level) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_recommended_level");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_recommended_level = recommended_level;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_region_definition::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
