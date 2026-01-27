#include "document.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/lock_error_helpers.hpp"
#include "../PThread/pthread.hpp"

html_document::thread_guard::thread_guard(const html_document *document) noexcept
    : _document(document), _lock_acquired(false), _status(0)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (!this->_document)
        return ;
    this->_status = this->_document->lock(&this->_lock_acquired);
    if (this->_status == 0)
        ft_errno = FT_ERR_SUCCESSS;
    return ;
}

html_document::thread_guard::~thread_guard() noexcept
{
    if (!this->_document)
        return ;
    this->_document->unlock(this->_lock_acquired);
    return ;
}

int html_document::thread_guard::get_status() const noexcept
{
    return (this->_status);
}

bool html_document::thread_guard::lock_acquired() const noexcept
{
    return (this->_lock_acquired);
}

html_document::html_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr), _thread_safe_enabled(false),
      _operation_errors({{}, {}, 0})
{
    this->set_error(FT_ERR_SUCCESSS);
    if (this->prepare_thread_safety() != 0)
        return ;
    return ;
}

html_document::~html_document() noexcept
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

html_node *html_document::create_node(const char *tag_name, const char *text_content) noexcept
{
    html_node *node;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!tag_name)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    node = html_create_node(tag_name, text_content);
    if (!node)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->set_error(ft_errno);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (node);
}

html_attr *html_document::create_attr(const char *key, const char *value) noexcept
{
    html_attr *attribute;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!key || !value)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    attribute = html_create_attr(key, value);
    if (!attribute)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->set_error(ft_errno);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (attribute);
}

void html_document::add_attr(html_node *target_node, html_attr *new_attribute) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!target_node || !new_attribute)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_add_attr(target_node, new_attribute);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void html_document::remove_attr(html_node *target_node, const char *key) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!target_node || !key)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_attr(target_node, key);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void html_document::add_child(html_node *parent_node, html_node *child_node) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!parent_node || !child_node)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_add_child(parent_node, child_node);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void html_document::append_node(html_node *new_node) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!new_node)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_append_node(&this->_root, new_node);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int html_document::write_to_file(const char *file_path) const noexcept
{
    int result;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (-1);
    }
    if (!file_path)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    result = html_write_to_file(file_path, this->_root);
    if (result != 0)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_HANDLE);
        else
            const_cast<html_document *>(this)->set_error(ft_errno);
        return (-1);
    }
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (0);
}

char *html_document::write_to_string() const noexcept
{
    char *result;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    result = html_write_to_string(this->_root);
    if (!result)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            const_cast<html_document *>(this)->set_error(FT_ERR_NO_MEMORY);
        else
            const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

void html_document::remove_nodes_by_tag(const char *tag_name) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!tag_name)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_tag(&this->_root, tag_name);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void html_document::remove_nodes_by_attr(const char *key, const char *value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!key || !value)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_attr(&this->_root, key, value);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void html_document::remove_nodes_by_text(const char *text_content) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!text_content)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_text(&this->_root, text_content);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

html_node *html_document::find_by_tag(const char *tag_name) const noexcept
{
    html_node *node;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!tag_name)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_tag(this->_root, tag_name);
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (node);
}

html_node *html_document::find_by_attr(const char *key, const char *value) const noexcept
{
    html_node *node;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!key || !value)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_attr(this->_root, key, value);
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (node);
}

html_node *html_document::find_by_text(const char *text_content) const noexcept
{
    html_node *node;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!text_content)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_text(this->_root, text_content);
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (node);
}

html_node *html_document::find_by_selector(const char *selector) const noexcept
{
    html_node *node;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    if (!selector)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_selector(this->_root, selector);
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (node);
}

size_t html_document::count_nodes_by_tag(const char *tag_name) const noexcept
{
    size_t count;
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (0);
    }
    if (!tag_name)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    count = html_count_nodes_by_tag(this->_root, tag_name);
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (count);
}

html_node *html_document::get_root() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<html_document *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_root);
}

int html_document::get_error() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (ft_errno);
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

const char *html_document::get_error_str() const noexcept
{
    int error_code = this->get_error();
    const char *message = ft_strerror(error_code);

    if (message == ft_nullptr)
        message = "unknown error";
    return (message);
}

void html_document::clear() noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    html_free_nodes(this->_root);
    this->_root = ft_nullptr;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

bool html_document::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (false);
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
        return (false);
    }
    const_cast<html_document *>(this)->set_error(FT_ERR_SUCCESSS);
    return (true);
}

void html_document::set_error(int error_code) const noexcept
{
    this->record_operation_error(error_code);
    ft_errno = error_code;
    return ;
}

void html_document::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id = ft_errno_next_operation_id();

    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

pt_mutex *html_document::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}

ft_operation_error_stack *html_document::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

int html_document::prepare_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (!memory)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    int mutex_error = ft_mutex_pop_last_error(mutex_pointer);

    if (mutex_error != FT_ERR_SUCCESSS)
    {
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

void html_document::teardown_thread_safety() noexcept
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

int html_document::lock(bool *lock_acquired) const noexcept
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    int mutex_error = ft_mutex_pop_last_error(this->_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_errno = mutex_error;
        const_cast<html_document *>(this)->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void html_document::unlock(bool lock_acquired) const noexcept
{
    ft_errno = FT_ERR_SUCCESSS;
    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    int mutex_error = ft_mutex_pop_last_error(this->_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_errno = mutex_error;
        const_cast<html_document *>(this)->set_error(mutex_error);
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
