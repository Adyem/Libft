#include "dom.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_dom_document::ft_dom_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr),
      _initialized_state(ft_dom_document::_state_uninitialized)
{
    return ;
}

ft_dom_document::~ft_dom_document() noexcept
{
    if (this->_initialized_state == ft_dom_document::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dom_document::~ft_dom_document",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_dom_document::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dom_document::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dom_document lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_dom_document::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dom_document::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dom_document::initialize() noexcept
{
    if (this->_initialized_state == ft_dom_document::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_document::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_root = ft_nullptr;
    this->_initialized_state = ft_dom_document::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dom_document::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_dom_document::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_document::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dom_document::_state_destroyed;
    return (disable_error);
}

int ft_dom_document::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dom_document::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
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

int ft_dom_document::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dom_document::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_dom_document::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_dom_document::lock_internal(bool *lock_acquired) const noexcept
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

int ft_dom_document::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dom_document::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_document::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_document::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_document::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_dom_document::set_root(ft_dom_node *root) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_document::set_root");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_root != ft_nullptr)
        delete this->_root;
    this->_root = root;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_dom_node *ft_dom_document::get_root() const noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_dom_node *root_pointer;

    this->abort_if_not_initialized("ft_dom_document::get_root");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    root_pointer = this->_root;
    (void)this->unlock_internal(lock_acquired);
    return (root_pointer);
}

void ft_dom_document::clear() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_document::clear");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_root != ft_nullptr)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_dom_document::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dom_document::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
