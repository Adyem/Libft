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

ft_dom_node::ft_dom_node() noexcept
    : _type(FT_DOM_NODE_NULL), _name(), _value(), _children(),
      _attribute_keys(), _attribute_values(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_dom_node::ft_dom_node(const ft_dom_node &other) noexcept
    : _type(other._type), _name(other._name), _value(other._value),
      _children(other._children), _attribute_keys(other._attribute_keys),
      _attribute_values(other._attribute_values), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}

ft_dom_node::ft_dom_node(ft_dom_node &&other) noexcept
    : _type(other._type), _name(other._name), _value(other._value),
      _children(other._children), _attribute_keys(other._attribute_keys),
      _attribute_values(other._attribute_values), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}

ft_dom_node::~ft_dom_node() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_dom_node::initialize() noexcept
{
    int32_t initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_dom_node::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_type = FT_DOM_NODE_NULL;

    if (this->_children.is_initialised() == FT_CLASS_STATE_INITIALISED)
    {
        this->_children.clear();
        if (this->_children.get_error() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_children.get_error());
        }
    }
    else
    {
        initialize_error = this->_children.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    if (this->_attribute_keys.is_initialised() == FT_CLASS_STATE_INITIALISED)
    {
        this->_attribute_keys.clear();
        if (this->_attribute_keys.get_error() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_attribute_keys.get_error());
        }
    }
    else
    {
        initialize_error = this->_attribute_keys.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    if (this->_attribute_values.is_initialised() == FT_CLASS_STATE_INITIALISED)
    {
        this->_attribute_values.clear();
        if (this->_attribute_values.get_error() != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_attribute_values.get_error());
        }
    }
    else
    {
        initialize_error = this->_attribute_values.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    if (this->_name.is_initialised() == FT_TRUE)
    {
        initialize_error = this->_name.clear();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    else
    {
        initialize_error = this->_name.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    if (this->_value.is_initialised() == FT_TRUE)
    {
        initialize_error = this->_value.clear();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    else
    {
        initialize_error = this->_value.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (initialize_error);
        }
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_node::destroy() noexcept
{
    ft_size_t child_index;
    ft_size_t child_count;
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
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
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_dom_node::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::enable_thread_safety");
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

int32_t ft_dom_node::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_dom_node::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_dom_node::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t ft_dom_node::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_node::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_node::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_dom_node_type ft_dom_node::get_type() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_dom_node_type type;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_type");
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
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::set_type");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = type;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t ft_dom_node::set_name(const ft_string &name) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::set_name(ft_string)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_name = name;
    (void)this->unlock_internal(lock_acquired);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_node::set_name(const char *name) noexcept
{
    ft_string name_string;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::set_name(const char *)");
    if (name == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    name_string = name;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (this->set_name(name_string));
}

const ft_string &ft_dom_node::get_name() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_name");
    return (this->_name);
}

int32_t ft_dom_node::set_value(const ft_string &value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::set_value(ft_string)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_value = value;
    (void)this->unlock_internal(lock_acquired);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_node::set_value(const char *value) noexcept
{
    ft_string value_string;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::set_value(const char *)");
    if (value == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    value_string = value;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (this->set_value(value_string));
}

const ft_string &ft_dom_node::get_value() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_value");
    return (this->_value);
}

int32_t ft_dom_node::add_child(ft_dom_node *child) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::add_child");
    if (child == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_children.push_back(child);
    (void)this->unlock_internal(lock_acquired);
    return (this->_children.get_error());
}

const ft_vector<ft_dom_node*> &ft_dom_node::get_children() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_children");
    return (this->_children);
}

int32_t ft_dom_node::add_attribute(const ft_string &key, const ft_string &value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t key_index;
    ft_size_t key_count;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::add_attribute(ft_string,ft_string)");
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
            return (ft_string::get_error());
        }
        key_index += 1;
    }
    this->_attribute_keys.push_back(key);
    if (this->_attribute_keys.get_error() != FT_ERR_SUCCESS)
    {
        (void)this->unlock_internal(lock_acquired);
        return (this->_attribute_keys.get_error());
    }
    this->_attribute_values.push_back(value);
    (void)this->unlock_internal(lock_acquired);
    return (this->_attribute_values.get_error());
}

int32_t ft_dom_node::add_attribute(const char *key, const char *value) noexcept
{
    ft_string key_string;
    ft_string value_string;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::add_attribute(const char *,const char *)");
    if (key == ft_nullptr || value == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    key_string = key;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    value_string = value;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    return (this->add_attribute(key_string, value_string));
}

ft_bool ft_dom_node::has_attribute(const ft_string &key) const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t key_index;
    ft_size_t key_count;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::has_attribute");
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
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t value_initialize_error;
    ft_size_t key_index;
    ft_size_t key_count;
    ft_string value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_attribute");
    value_initialize_error = value.initialize();
    if (value_initialize_error != FT_ERR_SUCCESS)
        return (ft_string::from_error(value_initialize_error));
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_attribute_keys");
    return (this->_attribute_keys);
}

const ft_vector<ft_string> &ft_dom_node::get_attribute_values() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::get_attribute_values");
    return (this->_attribute_values);
}

ft_dom_node *ft_dom_node::find_child(const ft_string &name) const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t child_index;
    ft_size_t child_count;
    ft_dom_node *child;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_node::find_child");
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
