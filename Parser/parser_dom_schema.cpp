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

ft_dom_schema::ft_dom_schema() noexcept
    : _rules(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_dom_schema::ft_dom_schema(const ft_dom_schema &other) noexcept
    : _rules(other._rules), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}

ft_dom_schema::ft_dom_schema(ft_dom_schema &&other) noexcept
    : _rules(other._rules), _mutex(ft_nullptr),
      _initialised_state(other._initialised_state)
{
    return ;
}

ft_dom_schema::~ft_dom_schema() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_dom_schema::initialize() noexcept
{
    int32_t member_initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_dom_schema::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    member_initialize_error = this->_rules.initialize();
    if (member_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (member_initialize_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_schema::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_rules.clear();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_dom_schema::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::enable_thread_safety");
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

int32_t ft_dom_schema::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_dom_schema::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_dom_schema::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t ft_dom_schema::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_schema::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dom_schema::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t ft_dom_schema::add_rule(const ft_string &path, ft_dom_node_type type, ft_bool required) noexcept
{
    ft_dom_schema_rule rule;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::add_rule");
    rule.path = path;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    rule.type = type;
    rule.required = required;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_rules.push_back(rule);
    (void)this->unlock_internal(lock_acquired);
    if (this->_rules.get_error() != FT_ERR_SUCCESS)
        return (static_cast<int32_t>(this->_rules.get_error()));
    return (FT_ERR_SUCCESS);
}

static ft_string ft_dom_build_path(const ft_string &base, const ft_string &segment) noexcept
{
    ft_string result;

    result = base;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string());
    if (result.size() != 0)
        result += "/";
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string());
    result += segment;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string());
    return (result);
}

int32_t ft_dom_schema::validate_rule(const ft_dom_schema_rule &rule, const ft_dom_node *node,
    const ft_string &base_path, ft_dom_validation_report &report) const noexcept
{
    ft_string full_path;
    const ft_dom_node *target_node;
    int32_t find_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::validate_rule");
    full_path = ft_dom_build_path(base_path, rule.path);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (ft_string::get_error());
    target_node = ft_nullptr;
    find_error = ft_dom_find_path(node, full_path, &target_node);
    if (find_error != FT_ERR_SUCCESS)
    {
        if (rule.required)
        {
            ft_string message;
            if (message.initialize("Required node missing") != FT_ERR_SUCCESS)
                return (ft_string::get_error());

            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (ft_string::get_error());
            report.add_error(full_path, message);
            report.mark_invalid();
        }
        return (find_error);
    }
    if (!target_node)
    {
        if (rule.required)
        {
            ft_string message;
            if (message.initialize("Required node missing") != FT_ERR_SUCCESS)
                return (ft_string::get_error());

            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (ft_string::get_error());
            report.add_error(full_path, message);
            report.mark_invalid();
        }
        return (FT_ERR_SUCCESS);
    }
    ft_dom_node_type node_type;

    node_type = target_node->get_type();
    if (node_type != rule.type)
    {
        ft_string message;
        if (message.initialize("Node type mismatch") != FT_ERR_SUCCESS)
            return (ft_string::get_error());

        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (ft_string::get_error());
        report.add_error(full_path, message);
        report.mark_invalid();
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_dom_schema::validate(const ft_dom_document &document, ft_dom_validation_report &report) const noexcept
{
    const ft_dom_node *root;
    ft_size_t index;
    ft_size_t count;
    int32_t rule_error;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_dom_schema::validate");
    report.mark_valid();
    root = document.get_root();
    if (!root)
    {
        report.mark_invalid();
        ft_string path;
        if (path.initialize("<root>") != FT_ERR_SUCCESS)
            return (ft_string::get_error());
        ft_string message;
        if (message.initialize("Document has no root node") != FT_ERR_SUCCESS)
            return (ft_string::get_error());
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

        if (this->_rules.get_error() != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            return (static_cast<int32_t>(this->_rules.get_error()));
        }
        ft_string rule_path;
        if (rule_path.initialize("") != FT_ERR_SUCCESS)
        {
            (void)this->unlock_internal(lock_acquired);
            return (ft_string::get_error());
        }
        rule_error = this->validate_rule(rule, root, rule_path, report);
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
