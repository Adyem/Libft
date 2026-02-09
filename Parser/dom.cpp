#include "dom.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
#include <new>

ft_dom_node::thread_guard::thread_guard(const ft_dom_node *node) noexcept
    : _node(node), _lock_acquired(false), _status(0)
{
    if (!this->_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    this->_status = this->_node->lock(&this->_lock_acquired);
    if (this->_status == 0)
        ft_errno = FT_ERR_SUCCESSS;
    return ;
}

ft_dom_node::thread_guard::~thread_guard() noexcept
{
    if (!this->_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    this->_node->unlock(this->_lock_acquired);
    return ;
}

int ft_dom_node::thread_guard::get_status() const noexcept
{
    return (this->_status);
}

bool ft_dom_node::thread_guard::lock_acquired() const noexcept
{
    return (this->_lock_acquired);
}

ft_dom_node::ft_dom_node() noexcept
    : _type(FT_DOM_NODE_NULL), _name(), _value(),
    _children(), _attribute_keys(), _attribute_values(), _operation_errors({{}, {}, 0}),
    _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->_name = "";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return ;
    }
    this->_value = "";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return ;
    }
    if (this->prepare_thread_safety() != 0)
    {
        this->record_operation_error(ft_errno);
        return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_node::~ft_dom_node() noexcept
{
    {
        thread_guard guard(this);

        if (guard.get_status() == 0)
        {
            size_t index;
            size_t count;

            index = 0;
            count = this->_children.size();
            if (this->_children.get_error() != FT_ERR_SUCCESSS)
                this->record_operation_error(this->_children.get_error());
            else
            {
                while (index < count)
                {
                    ft_dom_node *child;

                    child = this->_children[index];
                    if (this->_children.get_error() != FT_ERR_SUCCESSS)
                    {
                        this->record_operation_error(this->_children.get_error());
                        break ;
                    }
                    delete child;
                    index += 1;
                }
            }
            this->_children.clear();
            if (this->_children.get_error() != FT_ERR_SUCCESSS)
                this->record_operation_error(this->_children.get_error());
            this->_attribute_keys.clear();
            if (this->_attribute_keys.get_error() != FT_ERR_SUCCESSS)
                this->record_operation_error(this->_attribute_keys.get_error());
            this->_attribute_values.clear();
            if (this->_attribute_values.get_error() != FT_ERR_SUCCESSS)
                this->record_operation_error(this->_attribute_values.get_error());
        }
        else
            this->record_operation_error(ft_errno);
    }
    this->teardown_thread_safety();
    return ;
}

void ft_dom_node::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors,
            error_code, operation_id);
    ft_errno = error_code;
    return ;
}

int ft_dom_node::prepare_thread_safety() noexcept
{
    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    memory_pointer = cma_malloc(sizeof(pt_mutex));
    if (!memory_pointer)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory_pointer);
        this->record_operation_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

void ft_dom_node::teardown_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = false;
        return ;
    }
    this->_mutex->~pt_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    return ;
}

int ft_dom_node::lock(bool *lock_acquired) const noexcept
{
    int mutex_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = this->_mutex->get_error();
    if (mutex_error == FT_ERR_SUCCESSS)
    {
        if (lock_acquired)
            *lock_acquired = true;
        const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    ft_errno = mutex_error;
    const_cast<ft_dom_node *>(this)->record_operation_error(mutex_error);
    return (-1);
}

void ft_dom_node::unlock(bool lock_acquired) const noexcept
{
    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<ft_dom_node *>(this)->record_operation_error(mutex_error);
        return ;
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_node_type ft_dom_node::get_type() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_type);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_type);
}

void ft_dom_node::set_type(ft_dom_node_type type) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = type;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_dom_node::set_name(const ft_string &name) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (-1);
    this->_name = name;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_dom_node::set_name(const char *name) noexcept
{
    if (!name)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_string temp(name);

    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    return (this->set_name(temp));
}

const ft_string &ft_dom_node::get_name() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_name);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_name);
}

int ft_dom_node::set_value(const ft_string &value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (-1);
    this->_value = value;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_dom_node::set_value(const char *value) noexcept
{
    if (!value)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_string temp(value);

    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    return (this->set_value(temp));
}

const ft_string &ft_dom_node::get_value() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_value);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_value);
}

int ft_dom_node::add_child(ft_dom_node *child) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (-1);
    if (!child)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    this->_children.push_back(child);
    if (this->_children.get_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(this->_children.get_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

const ft_vector<ft_dom_node*> &ft_dom_node::get_children() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_children);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_children);
}

int ft_dom_node::add_attribute(const ft_string &key, const ft_string &value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (-1);
    size_t index;
    size_t count;

    index = 0;
    count = this->_attribute_keys.size();
    while (index < count)
    {
        const ft_string &existing_key = this->_attribute_keys[index];

        if (this->_attribute_keys.get_error() != FT_ERR_SUCCESSS)
        {
            this->record_operation_error(this->_attribute_keys.get_error());
            return (-1);
        }
        if (existing_key == key)
        {
            ft_string &existing_value = this->_attribute_values[index];

            if (this->_attribute_values.get_error() != FT_ERR_SUCCESSS)
            {
                this->record_operation_error(this->_attribute_values.get_error());
                return (-1);
            }
            existing_value = value;
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                this->record_operation_error(ft_string::last_operation_error());
                return (-1);
            }
            this->record_operation_error(FT_ERR_SUCCESSS);
            return (0);
        }
        index += 1;
    }
    this->_attribute_keys.push_back(key);
    if (this->_attribute_keys.get_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(this->_attribute_keys.get_error());
        return (-1);
    }
    this->_attribute_values.push_back(value);
    if (this->_attribute_values.get_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(this->_attribute_values.get_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_dom_node::add_attribute(const char *key, const char *value) noexcept
{
    if (!key || !value)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_string key_string(key);

    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    ft_string value_string(value);

    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    return (this->add_attribute(key_string, value_string));
}

bool ft_dom_node::has_attribute(const ft_string &key) const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (false);
    }
    size_t index;
    size_t count;

    index = 0;
    count = this->_attribute_keys.size();
    while (index < count)
    {
        const ft_string &existing_key = this->_attribute_keys[index];

        if (this->_attribute_keys.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(this->_attribute_keys.get_error());
            return (false);
        }
        if (existing_key == key)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
            return (true);
        }
        index += 1;
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (false);
}

ft_string ft_dom_node::get_attribute(const ft_string &key) const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (ft_string(ft_errno));
    }
    size_t index;
    size_t count;

    index = 0;
    count = this->_attribute_keys.size();
    while (index < count)
    {
        const ft_string &existing_key = this->_attribute_keys[index];

        if (this->_attribute_keys.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(this->_attribute_keys.get_error());
            return (ft_string(this->_attribute_keys.get_error()));
        }
        if (existing_key == key)
        {
            const ft_string &stored_value = this->_attribute_values[index];
            ft_string result(stored_value);

            if (this->_attribute_values.get_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_node *>(this)->record_operation_error(this->_attribute_values.get_error());
                return (ft_string(this->_attribute_values.get_error()));
            }
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_node *>(this)->record_operation_error(
                    ft_string::last_operation_error());
                return (ft_string(ft_string::last_operation_error()));
            }
            const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
            return (result);
        }
        index += 1;
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_NOT_FOUND);
    return (ft_string(FT_ERR_NOT_FOUND));
}

const ft_vector<ft_string> &ft_dom_node::get_attribute_keys() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_attribute_keys);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_attribute_keys);
}

const ft_vector<ft_string> &ft_dom_node::get_attribute_values() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (this->_attribute_values);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_attribute_values);
}

ft_dom_node *ft_dom_node::find_child(const ft_string &name) const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(ft_errno);
        return (ft_nullptr);
    }
    size_t index;
    size_t count;

    index = 0;
    count = this->_children.size();
    if (this->_children.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(this->_children.get_error());
        return (ft_nullptr);
    }
    while (index < count)
    {
        ft_dom_node *child;

        child = this->_children[index];
        if (this->_children.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(this->_children.get_error());
            return (ft_nullptr);
        }
        const ft_string &child_name = child->get_name();

        if (child->get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(child->get_error());
            return (ft_nullptr);
        }
        if (child_name == name)
        {
            const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
            return (child);
        }
        index += 1;
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_NOT_FOUND);
    return (ft_nullptr);
}

bool ft_dom_node::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (false);
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        return (false);
    }
    const_cast<ft_dom_node *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (true);
}

int ft_dom_node::get_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

pt_mutex *ft_dom_node::mutex_handle() const noexcept
{
    return (this->_mutex);
}

ft_operation_error_stack *ft_dom_node::operation_error_stack_handle() noexcept
{
    return (&this->_operation_errors);
}

const ft_operation_error_stack *ft_dom_node::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

ft_dom_document::thread_guard::thread_guard(const ft_dom_document *document) noexcept
    : _document(document), _lock_acquired(false), _status(0)
{
    if (!this->_document)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    this->_status = this->_document->lock(&this->_lock_acquired);
    if (this->_status == 0)
        ft_errno = FT_ERR_SUCCESSS;
    return ;
}

ft_dom_document::thread_guard::~thread_guard() noexcept
{
    if (!this->_document)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    this->_document->unlock(this->_lock_acquired);
    return ;
}

int ft_dom_document::thread_guard::get_status() const noexcept
{
    return (this->_status);
}

bool ft_dom_document::thread_guard::lock_acquired() const noexcept
{
    return (this->_lock_acquired);
}

ft_dom_document::ft_dom_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr), _thread_safe_enabled(false), _operation_errors({{}, {}, 0})
{
    if (this->prepare_thread_safety() != 0)
    {
        this->record_operation_error(ft_errno);
        return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_document::~ft_dom_document() noexcept
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

void ft_dom_document::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors,
            error_code, operation_id);
    ft_errno = error_code;
    return ;
}

int ft_dom_document::prepare_thread_safety() noexcept
{
    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    memory_pointer = cma_malloc(sizeof(pt_mutex));
    if (!memory_pointer)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory_pointer);
        this->record_operation_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

void ft_dom_document::teardown_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = false;
        return ;
    }
    this->_mutex->~pt_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    return ;
}

int ft_dom_document::lock(bool *lock_acquired) const noexcept
{
    int mutex_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = this->_mutex->get_error();
    if (mutex_error == FT_ERR_SUCCESSS)
    {
        if (lock_acquired)
            *lock_acquired = true;
        const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    ft_errno = mutex_error;
    const_cast<ft_dom_document *>(this)->record_operation_error(mutex_error);
    return (-1);
}

void ft_dom_document::unlock(bool lock_acquired) const noexcept
{
    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<ft_dom_document *>(this)->record_operation_error(mutex_error);
        return ;
    }
    const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_dom_document::set_root(ft_dom_node *root) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    if (this->_root)
        delete this->_root;
    this->_root = root;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_node *ft_dom_document::get_root() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<ft_dom_document *>(this)->record_operation_error(ft_errno);
        return (ft_nullptr);
    }
    const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (this->_root);
}

void ft_dom_document::clear() noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    if (this->_root)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_dom_document::get_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

const char *ft_dom_document::get_error_str() const noexcept
{
    return (ft_strerror(ft_operation_error_stack_last_error(&this->_operation_errors)));
}

pt_mutex *ft_dom_document::mutex_handle() const noexcept
{
    return (this->_mutex);
}

ft_operation_error_stack *ft_dom_document::operation_error_stack_handle() noexcept
{
    return (&this->_operation_errors);
}

const ft_operation_error_stack *ft_dom_document::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

bool ft_dom_document::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (false);
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        return (false);
    }
    const_cast<ft_dom_document *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (true);
}

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

void ft_dom_validation_report::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors,
            error_code, operation_id);
    ft_errno = error_code;
    return ;
}

ft_dom_validation_report::ft_dom_validation_report() noexcept
    : _valid(true), _errors(), _operation_errors({{}, {}, 0})
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_validation_report::~ft_dom_validation_report() noexcept
{
    size_t index;
    size_t count;

    index = 0;
    count = this->_errors.size();
    while (index < count)
    {
        ft_dom_validation_error &error_reference = this->_errors[index];

        (void)error_reference;
        index += 1;
    }
    this->_errors.clear();
    return ;
}

void ft_dom_validation_report::mark_valid() noexcept
{
    this->_valid = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_dom_validation_report::mark_invalid() noexcept
{
    this->_valid = false;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

bool ft_dom_validation_report::valid() const noexcept
{
    return (this->_valid);
}

int ft_dom_validation_report::get_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

const char *ft_dom_validation_report::get_error_str() const noexcept
{
    return (ft_strerror(ft_operation_error_stack_last_error(&this->_operation_errors)));
}

const ft_operation_error_stack *ft_dom_validation_report::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

int ft_dom_validation_report::add_error(const ft_string &path, const ft_string &message) noexcept
{
    ft_dom_validation_error error_entry;

    error_entry.path = path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    error_entry.message = message;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    this->_errors.push_back(error_entry);
    if (this->_errors.get_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(this->_errors.get_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

const ft_vector<ft_dom_validation_error> &ft_dom_validation_report::errors() const noexcept
{
    return (this->_errors);
}

void ft_dom_schema::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors,
            error_code, operation_id);
    ft_errno = error_code;
    return ;
}

const ft_operation_error_stack *ft_dom_schema::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

ft_dom_schema::ft_dom_schema() noexcept
    : _rules(), _operation_errors({{}, {}, 0})
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_dom_schema::~ft_dom_schema() noexcept
{
    this->_rules.clear();
    return ;
}

int ft_dom_schema::add_rule(const ft_string &path, ft_dom_node_type type, bool required) noexcept
{
    ft_dom_schema_rule rule;

    rule.path = path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    rule.type = type;
    rule.required = required;
    this->_rules.push_back(rule);
    if (this->_rules.get_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(this->_rules.get_error());
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

static ft_string ft_dom_build_path(const ft_string &base, const ft_string &segment) noexcept
{
    ft_string result;

    result = base;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (ft_string(ft_string::last_operation_error()));
    if (result.size() != 0)
        result += "/";
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (ft_string(ft_string::last_operation_error()));
    result += segment;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (ft_string(ft_string::last_operation_error()));
    return (result);
}

int ft_dom_schema::validate_rule(const ft_dom_schema_rule &rule, const ft_dom_node *node,
    const ft_string &base_path, ft_dom_validation_report &report) const noexcept
{
    ft_string full_path;
    const ft_dom_node *target_node;

    full_path = ft_dom_build_path(base_path, rule.path);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_dom_schema *>(this)->record_operation_error(ft_string::last_operation_error());
        return (-1);
    }
    target_node = ft_nullptr;
    if (ft_dom_find_path(node, full_path, &target_node) != 0)
    {
        if (rule.required)
        {
            ft_string message("Required node missing");

            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_schema *>(this)->record_operation_error(ft_string::last_operation_error());
                return (-1);
            }
            report.add_error(full_path, message);
            if (report.get_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_schema *>(this)->record_operation_error(report.get_error());
                return (-1);
            }
            report.mark_invalid();
        }
        const_cast<ft_dom_schema *>(this)->record_operation_error(ft_errno);
        return (0);
    }
    if (!target_node)
    {
        if (rule.required)
        {
            ft_string message("Required node missing");

            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_schema *>(this)->record_operation_error(ft_string::last_operation_error());
                return (-1);
            }
            report.add_error(full_path, message);
            if (report.get_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_dom_schema *>(this)->record_operation_error(report.get_error());
                return (-1);
            }
            report.mark_invalid();
        }
        const_cast<ft_dom_schema *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    ft_dom_node_type node_type;

    node_type = target_node->get_type();
    if (target_node->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_dom_schema *>(this)->record_operation_error(target_node->get_error());
        return (-1);
    }
    if (node_type != rule.type)
    {
        ft_string message("Node type mismatch");

        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_schema *>(this)->record_operation_error(ft_string::last_operation_error());
            return (-1);
        }
        report.add_error(full_path, message);
        if (report.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_schema *>(this)->record_operation_error(report.get_error());
            return (-1);
        }
        report.mark_invalid();
    }
    const_cast<ft_dom_schema *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_dom_schema::validate(const ft_dom_document &document, ft_dom_validation_report &report) const noexcept
{
    const ft_dom_node *root;
    size_t index;
    size_t count;

    report.mark_valid();
    root = document.get_root();
    if (document.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_dom_schema *>(this)->record_operation_error(document.get_error());
        return (-1);
    }
    if (!root)
    {
        report.mark_invalid();
        ft_string path("<root>");
        ft_string message("Document has no root node");

        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS || ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_schema *>(this)->record_operation_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        report.add_error(path, message);
        if (report.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_schema *>(this)->record_operation_error(report.get_error());
            return (-1);
        }
        return (0);
    }
    index = 0;
    count = this->_rules.size();
    while (index < count)
    {
        const ft_dom_schema_rule &rule = this->_rules[index];

        if (this->_rules.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_dom_schema *>(this)->record_operation_error(this->_rules.get_error());
            return (-1);
        }
        if (this->validate_rule(rule, root, ft_string(""), report) != 0)
        {
            if (ft_operation_error_stack_last_error(&this->_operation_errors) != FT_ERR_SUCCESSS)
                return (-1);
        }
        index += 1;
    }
    const_cast<ft_dom_schema *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_dom_find_path(const ft_dom_node *root, const ft_string &path, const ft_dom_node **out_node) noexcept
{
    if (!root || !out_node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *out_node = ft_nullptr;
    const char *raw_path;

    raw_path = path.c_str();
    if (!raw_path)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    size_t path_length;

    path_length = path.size();
    if (path_length == 0)
    {
        *out_node = root;
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    const ft_dom_node *current_node;
    size_t start_index;
    int status;
    int error_code;

    current_node = root;
    start_index = 0;
    status = 0;
    error_code = FT_ERR_SUCCESSS;
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
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                status = -1;
                error_code = ft_string::last_operation_error();
            }
            else
            {
                ft_dom_node *child_node;

                child_node = current_node->find_child(segment);
                int node_error;

                node_error = current_node->get_error();
                if (node_error != FT_ERR_SUCCESSS)
                {
                    status = -1;
                    error_code = node_error;
                }
                else if (!child_node)
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
    {
        ft_errno = error_code;
        return (-1);
    }
    *out_node = current_node;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}
