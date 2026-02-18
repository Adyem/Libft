#include "dom.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_dom_node::ft_dom_node() noexcept
    : _type(FT_DOM_NODE_NULL), _name(), _value(), _children(),
      _attribute_keys(), _attribute_values(), _mutex(ft_nullptr),
      _initialized_state(ft_dom_node::_state_uninitialized)
{
    return ;
}

ft_dom_node::~ft_dom_node() noexcept
{
    if (this->_initialized_state == ft_dom_node::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dom_node::~ft_dom_node",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_dom_node::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dom_node::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dom_node lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_dom_node::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dom_node::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dom_node::initialize() noexcept
{
    if (this->_initialized_state == ft_dom_node::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_node::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_type = FT_DOM_NODE_NULL;
    this->_children.clear();
    this->_attribute_keys.clear();
    this->_attribute_values.clear();
    this->_name = "";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_dom_node::_state_destroyed;
        return (ft_string::last_operation_error());
    }
    this->_value = "";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_dom_node::_state_destroyed;
        return (ft_string::last_operation_error());
    }
    this->_initialized_state = ft_dom_node::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dom_node::destroy() noexcept
{
    size_t child_index;
    size_t child_count;
    int disable_error;

    if (this->_initialized_state != ft_dom_node::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_node::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    child_index = 0;
    child_count = this->_children.size();
    while (child_index < child_count)
    {
        delete this->_children[child_index];
        child_index += 1;
    }
    this->_children.clear();
    this->_attribute_keys.clear();
    this->_attribute_values.clear();
    this->_name = "";
    this->_value = "";
    this->_type = FT_DOM_NODE_NULL;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dom_node::_state_destroyed;
    return (disable_error);
}

int ft_dom_node::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dom_node::enable_thread_safety");
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

int ft_dom_node::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dom_node::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_dom_node::lock_internal(bool *lock_acquired) const noexcept
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

int ft_dom_node::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dom_node::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_node::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_dom_node_type ft_dom_node::get_type() const noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_dom_node_type type;

    this->abort_if_not_initialized("ft_dom_node::get_type");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_DOM_NODE_NULL);
    type = this->_type;
    (void)this->unlock_internal(lock_acquired);
    return (type);
}

void ft_dom_node::set_type(ft_dom_node_type type) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_node::set_type");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = type;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_dom_node::set_name(const ft_string &name) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_node::set_name(ft_string)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_name = name;
    (void)this->unlock_internal(lock_acquired);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

int ft_dom_node::set_name(const char *name) noexcept
{
    ft_string name_string;

    this->abort_if_not_initialized("ft_dom_node::set_name(const char *)");
    if (name == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    name_string = name;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (this->set_name(name_string));
}

const ft_string &ft_dom_node::get_name() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_name");
    return (this->_name);
}

int ft_dom_node::set_value(const ft_string &value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_node::set_value(ft_string)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_value = value;
    (void)this->unlock_internal(lock_acquired);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

int ft_dom_node::set_value(const char *value) noexcept
{
    ft_string value_string;

    this->abort_if_not_initialized("ft_dom_node::set_value(const char *)");
    if (value == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    value_string = value;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (this->set_value(value_string));
}

const ft_string &ft_dom_node::get_value() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_value");
    return (this->_value);
}

int ft_dom_node::add_child(ft_dom_node *child) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_node::add_child");
    if (child == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_children.push_back(child);
    (void)this->unlock_internal(lock_acquired);
    return (ft_vector<ft_dom_node *>::last_operation_error());
}

const ft_vector<ft_dom_node*> &ft_dom_node::get_children() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_children");
    return (this->_children);
}

int ft_dom_node::add_attribute(const ft_string &key, const ft_string &value) noexcept
{
    bool lock_acquired;
    int lock_error;
    size_t key_index;
    size_t key_count;

    this->abort_if_not_initialized("ft_dom_node::add_attribute(ft_string,ft_string)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    key_index = 0;
    key_count = this->_attribute_keys.size();
    while (key_index < key_count)
    {
        if (this->_attribute_keys[key_index] == key)
        {
            this->_attribute_values[key_index] = value;
            (void)this->unlock_internal(lock_acquired);
            return (ft_string::last_operation_error());
        }
        key_index += 1;
    }
    this->_attribute_keys.push_back(key);
    if (ft_vector<ft_string>::last_operation_error() != FT_ERR_SUCCESS)
    {
        (void)this->unlock_internal(lock_acquired);
        return (ft_vector<ft_string>::last_operation_error());
    }
    this->_attribute_values.push_back(value);
    (void)this->unlock_internal(lock_acquired);
    return (ft_vector<ft_string>::last_operation_error());
}

int ft_dom_node::add_attribute(const char *key, const char *value) noexcept
{
    ft_string key_string;
    ft_string value_string;

    this->abort_if_not_initialized("ft_dom_node::add_attribute(const char *,const char *)");
    if (key == ft_nullptr || value == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    key_string = key;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    value_string = value;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (this->add_attribute(key_string, value_string));
}

bool ft_dom_node::has_attribute(const ft_string &key) const noexcept
{
    bool lock_acquired;
    int lock_error;
    size_t key_index;
    size_t key_count;

    this->abort_if_not_initialized("ft_dom_node::has_attribute");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    key_index = 0;
    key_count = this->_attribute_keys.size();
    while (key_index < key_count)
    {
        if (this->_attribute_keys[key_index] == key)
        {
            (void)this->unlock_internal(lock_acquired);
            return (true);
        }
        key_index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return (false);
}

ft_string ft_dom_node::get_attribute(const ft_string &key) const noexcept
{
    bool lock_acquired;
    int lock_error;
    size_t key_index;
    size_t key_count;
    ft_string value;

    this->abort_if_not_initialized("ft_dom_node::get_attribute");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (value);
    key_index = 0;
    key_count = this->_attribute_keys.size();
    while (key_index < key_count)
    {
        if (this->_attribute_keys[key_index] == key)
        {
            value = this->_attribute_values[key_index];
            (void)this->unlock_internal(lock_acquired);
            return (value);
        }
        key_index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

const ft_vector<ft_string> &ft_dom_node::get_attribute_keys() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_attribute_keys");
    return (this->_attribute_keys);
}

const ft_vector<ft_string> &ft_dom_node::get_attribute_values() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_attribute_values");
    return (this->_attribute_values);
}

ft_dom_node *ft_dom_node::find_child(const ft_string &name) const noexcept
{
    bool lock_acquired;
    int lock_error;
    size_t child_index;
    size_t child_count;
    ft_dom_node *child;

    this->abort_if_not_initialized("ft_dom_node::find_child");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    child_index = 0;
    child_count = this->_children.size();
    while (child_index < child_count)
    {
        child = this->_children[child_index];
        if (child != ft_nullptr && child->get_name() == name)
        {
            (void)this->unlock_internal(lock_acquired);
            return (child);
        }
        child_index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return (ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_dom_node::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dom_node::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
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
    {
        this->abort_lifecycle_error("ft_dom_validation_report::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_errors.clear();
    this->_valid = true;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dom_validation_report::_state_destroyed;
    return (disable_error);
}

int ft_dom_validation_report::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dom_validation_report::enable_thread_safety");
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
    this->abort_if_not_initialized("ft_dom_validation_report::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_dom_validation_report::lock_internal(bool *lock_acquired) const noexcept
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

int ft_dom_validation_report::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
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
    if (this->_initialized_state == ft_dom_validation_report::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dom_validation_report::~ft_dom_validation_report",
            "destructor called while object is uninitialized");
        return ;
    }
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
pt_mutex *ft_dom_validation_report::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dom_validation_report::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_dom_schema::ft_dom_schema() noexcept
    : _rules(), _mutex(ft_nullptr),
      _initialized_state(ft_dom_schema::_state_uninitialized)
{
    return ;
}

ft_dom_schema::~ft_dom_schema() noexcept
{
    if (this->_initialized_state == ft_dom_schema::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dom_schema::~ft_dom_schema",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_dom_schema::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dom_schema::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dom_schema lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_dom_schema::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dom_schema::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dom_schema::initialize() noexcept
{
    if (this->_initialized_state == ft_dom_schema::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_schema::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_rules.clear();
    this->_initialized_state = ft_dom_schema::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dom_schema::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_dom_schema::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dom_schema::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_rules.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dom_schema::_state_destroyed;
    return (disable_error);
}

int ft_dom_schema::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dom_schema::enable_thread_safety");
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

int ft_dom_schema::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dom_schema::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_dom_schema::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_dom_schema::lock_internal(bool *lock_acquired) const noexcept
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

int ft_dom_schema::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dom_schema::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_schema::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_schema::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dom_schema::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_dom_schema::add_rule(const ft_string &path, ft_dom_node_type type, bool required) noexcept
{
    ft_dom_schema_rule rule;
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_schema::add_rule");
    rule.path = path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    rule.type = type;
    rule.required = required;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_rules.push_back(rule);
    (void)this->unlock_internal(lock_acquired);
    if (ft_vector<ft_dom_schema_rule>::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_vector<ft_dom_schema_rule>::last_operation_error());
    return (FT_ERR_SUCCESS);
}

static ft_string ft_dom_build_path(const ft_string &base, const ft_string &segment) noexcept
{
    ft_string result;

    result = base;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string(ft_string::last_operation_error()));
    if (result.size() != 0)
        result += "/";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string(ft_string::last_operation_error()));
    result += segment;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string(ft_string::last_operation_error()));
    return (result);
}

int ft_dom_schema::validate_rule(const ft_dom_schema_rule &rule, const ft_dom_node *node,
    const ft_string &base_path, ft_dom_validation_report &report) const noexcept
{
    ft_string full_path;
    const ft_dom_node *target_node;
    int find_error;

    this->abort_if_not_initialized("ft_dom_schema::validate_rule");
    full_path = ft_dom_build_path(base_path, rule.path);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    target_node = ft_nullptr;
    find_error = ft_dom_find_path(node, full_path, &target_node);
    if (find_error != FT_ERR_SUCCESS)
    {
        if (rule.required)
        {
            ft_string message("Required node missing");

            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                return (ft_string::last_operation_error());
            report.add_error(full_path, message);
            report.mark_invalid();
        }
        return (find_error);
    }
    if (!target_node)
    {
        if (rule.required)
        {
            ft_string message("Required node missing");

            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                return (ft_string::last_operation_error());
            report.add_error(full_path, message);
            report.mark_invalid();
        }
        return (FT_ERR_SUCCESS);
    }
    ft_dom_node_type node_type;

    node_type = target_node->get_type();
    if (node_type != rule.type)
    {
        ft_string message("Node type mismatch");

        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            return (ft_string::last_operation_error());
        report.add_error(full_path, message);
        report.mark_invalid();
    }
    return (FT_ERR_SUCCESS);
}

int ft_dom_schema::validate(const ft_dom_document &document, ft_dom_validation_report &report) const noexcept
{
    const ft_dom_node *root;
    size_t index;
    size_t count;
    int rule_error;
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_dom_schema::validate");
    report.mark_valid();
    root = document.get_root();
    if (!root)
    {
        report.mark_invalid();
        ft_string path("<root>");
        ft_string message("Document has no root node");

        if (ft_string::last_operation_error() != FT_ERR_SUCCESS || ft_string::last_operation_error() != FT_ERR_SUCCESS)
            return (FT_ERR_NO_MEMORY);
        report.add_error(path, message);
        return (FT_ERR_SUCCESS);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    index = 0;
    count = this->_rules.size();
    while (index < count)
    {
        const ft_dom_schema_rule &rule = this->_rules[index];

        if (ft_vector<ft_dom_schema_rule>::last_operation_error() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            return (ft_vector<ft_dom_schema_rule>::last_operation_error());
        }
        rule_error = this->validate_rule(rule, root, ft_string(""), report);
        if (rule_error != FT_ERR_SUCCESS && rule_error != FT_ERR_NOT_FOUND)
        {
            (void)this->unlock_internal(lock_acquired);
            return (rule_error);
        }
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_dom_schema::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dom_schema::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

int ft_dom_find_path(const ft_dom_node *root, const ft_string &path, const ft_dom_node **out_node) noexcept
{

    if (!root || !out_node)
        return (FT_ERR_INVALID_ARGUMENT);
    *out_node = ft_nullptr;
    const char *raw_path;

    raw_path = path.c_str();
    if (!raw_path)
        return (FT_ERR_INVALID_ARGUMENT);
    size_t path_length;

    path_length = path.size();
    if (path_length == 0)
    {
        *out_node = root;
        return (FT_ERR_SUCCESS);
    }
    const ft_dom_node *current_node;
    size_t start_index;
    int status;
    int error_code;

    current_node = root;
    start_index = 0;
    status = 0;
    error_code = FT_ERR_SUCCESS;
    while (start_index < path_length && status == 0)
    {
        size_t end_index;
        size_t segment_length;

        end_index = start_index;
        while (end_index < path_length && raw_path[end_index] != '/')
            end_index += 1;
        segment_length = end_index - start_index;
        if (segment_length == 0)
        {
            status = -1;
            error_code = FT_ERR_INVALID_ARGUMENT;
        }
        else
        {
            ft_string segment;

            segment.assign(raw_path + start_index, segment_length);
            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            {
                status = -1;
                error_code = ft_string::last_operation_error();
            }
            else
            {
                ft_dom_node *child_node;

                child_node = current_node->find_child(segment);
                if (!child_node)
                {
                    status = -1;
                    error_code = FT_ERR_NOT_FOUND;
                }
                else
                {
                    current_node = child_node;
                    if (end_index >= path_length)
                        start_index = path_length;
                    else
                        start_index = end_index + 1;
                }
            }
        }
    }
    if (status != 0)
        return (error_code);
    *out_node = current_node;
    return (FT_ERR_SUCCESS);
}
