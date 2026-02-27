#include "dom.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_dom_schema_rule::ft_dom_schema_rule() noexcept
    : path(), type(FT_DOM_NODE_NULL), required(false)
{
    this->path = "";
    return ;
}

ft_dom_schema_rule::~ft_dom_schema_rule() noexcept
{
    return ;
}

ft_dom_validation_error::ft_dom_validation_error() noexcept
    : path(), message()
{
    this->path = "";
    this->message = "";
    return ;
}

ft_dom_validation_error::~ft_dom_validation_error() noexcept
{
    return ;
}

ft_dom_validation_report::ft_dom_validation_report() noexcept
    : _valid(true), _errors(), _mutex(ft_nullptr),
      _initialized_state(ft_dom_validation_report::_state_uninitialized)
{
    return ;
}

void ft_dom_validation_report::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dom_validation_report lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_dom_validation_report::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dom_validation_report::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dom_validation_report::initialize() noexcept
{
    if (this->_initialized_state == ft_dom_validation_report::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_validation_report::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_valid = true;
    this->_errors.clear();
    this->_initialized_state = ft_dom_validation_report::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dom_validation_report::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_dom_validation_report::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_errors.clear();
    this->_valid = true;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dom_validation_report::_state_destroyed;
    return (disable_error);
}

int ft_dom_validation_report::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dom_validation_report::enable_thread_safety");
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

int ft_dom_validation_report::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dom_validation_report::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_dom_validation_report::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_dom_validation_report::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_dom_validation_report::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_validation_report::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_validation_report::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_validation_report::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_dom_validation_report::~ft_dom_validation_report() noexcept
{
    if (this->_initialized_state == ft_dom_validation_report::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dom_validation_report::mark_valid() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_validation_report::mark_valid");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_valid = true;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_dom_validation_report::mark_invalid() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_validation_report::mark_invalid");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_valid = false;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

bool ft_dom_validation_report::valid() const noexcept
{
    bool lock_acquired;
    int lock_error;
    bool valid_value;

    this->abort_if_not_initialized("ft_dom_validation_report::valid");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    valid_value = this->_valid;
    (void)this->unlock_internal(lock_acquired);
    return (valid_value);
}

int ft_dom_validation_report::add_error(const ft_string &path,
    const ft_string &message) noexcept
{
    ft_dom_validation_error error_entry;
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_validation_report::add_error");
    error_entry.path = path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    error_entry.message = message;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_errors.push_back(error_entry);
    (void)this->unlock_internal(lock_acquired);
    if (ft_vector<ft_dom_validation_error>::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_vector<ft_dom_validation_error>::last_operation_error());
    return (FT_ERR_SUCCESS);
}

const ft_vector<ft_dom_validation_error> &ft_dom_validation_report::errors() const noexcept
{
    this->abort_if_not_initialized("ft_dom_validation_report::errors");
    return (this->_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_dom_validation_report::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dom_validation_report::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
