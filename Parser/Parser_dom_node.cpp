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
