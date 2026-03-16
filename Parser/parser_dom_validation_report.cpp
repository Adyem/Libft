#include "dom.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../Errno/errno_internal.hpp"
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
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_dom_validation_report::ft_dom_validation_report(
    const ft_dom_validation_report &other) noexcept
    : _valid(other._valid), _errors(other._errors), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}

ft_dom_validation_report::ft_dom_validation_report(
    ft_dom_validation_report &&other) noexcept
    : _valid(other._valid), _errors(other._errors), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}


int32_t ft_dom_validation_report::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_dom_validation_report::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_valid = true;
    this->_errors.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_validation_report::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_errors.clear();
    this->_valid = true;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_dom_validation_report::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::enable_thread_safety");
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

int32_t ft_dom_validation_report::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_dom_validation_report::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_dom_validation_report::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_validation_report::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_validation_report::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_validation_report::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_dom_validation_report::~ft_dom_validation_report() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

void ft_dom_validation_report::mark_valid() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::mark_valid");
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
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::mark_invalid");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_valid = false;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_bool ft_dom_validation_report::valid() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_bool valid_value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::valid");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    valid_value = this->_valid;
    (void)this->unlock_internal(lock_acquired);
    return (valid_value);
}

int32_t ft_dom_validation_report::add_error(const ft_string &path,
    const ft_string &message) noexcept
{
    ft_dom_validation_error error_entry;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::add_error");
    error_entry.path = path;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    error_entry.message = message;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_errors.push_back(error_entry);
    (void)this->unlock_internal(lock_acquired);
    if (this->_errors.get_error() != FT_ERR_SUCCESS)
        return (static_cast<int32_t>(this->_errors.get_error()));
    return (FT_ERR_SUCCESS);
}

const ft_vector<ft_dom_validation_error> &ft_dom_validation_report::errors() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_validation_report::errors");
    return (this->_errors);
}
