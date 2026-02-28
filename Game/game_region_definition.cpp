#include "../PThread/pthread_internal.hpp"
#include "ft_region_definition.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_region_definition::_last_error = FT_ERR_SUCCESS;

ft_region_definition::ft_region_definition() noexcept
    : _region_id(0), _name(), _description(), _recommended_level(0),
      _mutex(ft_nullptr),
      _initialized_state(ft_region_definition::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_region_definition::~ft_region_definition() noexcept
{
    if (this->_initialized_state != ft_region_definition::_state_initialized)
        return ;
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

void ft_region_definition::set_error(int error_code) const noexcept
{
    ft_region_definition::_last_error = error_code;
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
    this->set_error(FT_ERR_SUCCESS);
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
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    this->set_error(FT_ERR_SUCCESS);
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
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_region_id = region_id;
    this->_name = name;
    this->_description = description;
    this->_recommended_level = recommended_level;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_region_definition::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_region_definition::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_region_definition::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_region_definition::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_region_definition::enable_thread_safety");
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

int ft_region_definition::disable_thread_safety() noexcept
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

bool ft_region_definition::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_region_definition::lock_internal(bool *lock_acquired) const noexcept
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

int ft_region_definition::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_region_definition::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_region_definition::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_region_definition::get_region_id() const noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::get_region_id");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return (0);
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (this->_region_id);
}

void ft_region_definition::set_region_id(int region_id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_region_id");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    this->_region_id = region_id;
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

const ft_string &ft_region_definition::get_name() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_name");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_name);
}

void ft_region_definition::set_name(const ft_string &name) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_name");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    this->_name = name;
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

const ft_string &ft_region_definition::get_description() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_description");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_description);
}

void ft_region_definition::set_description(const ft_string &description) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_description");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    this->_description = description;
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_region_definition::get_recommended_level() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_recommended_level");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_recommended_level);
}

void ft_region_definition::set_recommended_level(int recommended_level) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_region_definition::set_recommended_level");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    this->_recommended_level = recommended_level;
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_region_definition::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_region_definition::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

int ft_region_definition::get_error() const noexcept
{
    return (ft_region_definition::_last_error);
}

const char *ft_region_definition::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
