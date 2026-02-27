#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

void html_document::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "html_document lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void html_document::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_state == html_document::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

html_document::html_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr),
    _state(html_document::_state_uninitialized)
{
    return ;
}

html_document::~html_document() noexcept
{
    if (this->_state == html_document::_state_initialized)
        (void)this->destroy();
    return ;
}

int html_document::initialize() noexcept
{
    if (this->_state == html_document::_state_initialized)
    {
        this->abort_lifecycle_error("html_document::initialize",
            "initialize called while already initialized");
    }
    this->_root = ft_nullptr;
    this->_state = html_document::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int html_document::destroy() noexcept
{
    int thread_safety_error;

    if (this->_state != html_document::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    html_free_nodes(this->_root);
    this->_root = ft_nullptr;
    thread_safety_error = this->disable_thread_safety();
    this->_state = html_document::_state_destroyed;
    return (thread_safety_error);
}

int html_document::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("html_document::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int html_document::disable_thread_safety() noexcept
{
    int destroy_error;

    this->abort_if_not_initialized("html_document::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool html_document::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

html_node *html_document::create_node(const char *tag_name,
    const char *text_content) noexcept
{
    int lock_error;
    html_node *node;

    this->abort_if_not_initialized("html_document::create_node");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (tag_name == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    node = html_create_node(tag_name, text_content);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node);
}

html_attr *html_document::create_attr(const char *key, const char *value) noexcept
{
    int lock_error;
    html_attr *attribute;

    this->abort_if_not_initialized("html_document::create_attr");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (key == ft_nullptr || value == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    attribute = html_create_attr(key, value);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (attribute);
}

void html_document::add_attr(html_node *target_node, html_attr *new_attribute) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::add_attr");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (target_node == ft_nullptr || new_attribute == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_add_attr(target_node, new_attribute);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void html_document::remove_attr(html_node *target_node, const char *key) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::remove_attr");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (target_node == ft_nullptr || key == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_remove_attr(target_node, key);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void html_document::add_child(html_node *parent_node, html_node *child_node) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::add_child");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (parent_node == ft_nullptr || child_node == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_add_child(parent_node, child_node);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void html_document::append_node(html_node *new_node) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::append_node");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (new_node == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_append_node(&this->_root, new_node);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int html_document::write_to_file(const char *file_path) const noexcept
{
    int lock_error;
    int write_error;

    this->abort_if_not_initialized("html_document::write_to_file");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (file_path == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (-1);
    }
    write_error = html_write_to_file(file_path, this->_root);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    if (write_error != 0)
        return (-1);
    return (0);
}

char *html_document::write_to_string() const noexcept
{
    int lock_error;
    char *result;

    this->abort_if_not_initialized("html_document::write_to_string");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    result = html_write_to_string(this->_root);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (result);
}

void html_document::remove_nodes_by_tag(const char *tag_name) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::remove_nodes_by_tag");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (tag_name == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_remove_nodes_by_tag(&this->_root, tag_name);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void html_document::remove_nodes_by_attr(const char *key, const char *value) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::remove_nodes_by_attr");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (key == ft_nullptr || value == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_remove_nodes_by_attr(&this->_root, key, value);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void html_document::remove_nodes_by_text(const char *text_content) noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::remove_nodes_by_text");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (text_content == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    html_remove_nodes_by_text(&this->_root, text_content);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

html_node *html_document::find_by_tag(const char *tag_name) const noexcept
{
    int lock_error;
    html_node *node;

    this->abort_if_not_initialized("html_document::find_by_tag");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (tag_name == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    node = html_find_by_tag(this->_root, tag_name);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node);
}

html_node *html_document::find_by_attr(const char *key, const char *value) const noexcept
{
    int lock_error;
    html_node *node;

    this->abort_if_not_initialized("html_document::find_by_attr");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (key == ft_nullptr || value == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    node = html_find_by_attr(this->_root, key, value);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node);
}

html_node *html_document::find_by_text(const char *text_content) const noexcept
{
    int lock_error;
    html_node *node;

    this->abort_if_not_initialized("html_document::find_by_text");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (text_content == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    node = html_find_by_text(this->_root, text_content);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node);
}

html_node *html_document::find_by_selector(const char *selector) const noexcept
{
    int lock_error;
    html_node *node;

    this->abort_if_not_initialized("html_document::find_by_selector");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (selector == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (ft_nullptr);
    }
    node = html_find_by_selector(this->_root, selector);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node);
}

size_t html_document::count_nodes_by_tag(const char *tag_name) const noexcept
{
    int lock_error;
    size_t node_count;

    this->abort_if_not_initialized("html_document::count_nodes_by_tag");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    if (tag_name == ft_nullptr)
    {
        (void)pt_mutex_unlock_if_not_null(this->_mutex);
        return (0);
    }
    node_count = html_count_nodes_by_tag(this->_root, tag_name);
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (node_count);
}

html_node *html_document::get_root() const noexcept
{
    int lock_error;
    html_node *root_node;

    this->abort_if_not_initialized("html_document::get_root");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    root_node = this->_root;
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return (root_node);
}

int html_document::get_error() const noexcept
{
    this->abort_if_not_initialized("html_document::get_error");
    return (FT_ERR_SUCCESS);
}

const char *html_document::get_error_str() const noexcept
{
    const char *message;

    this->abort_if_not_initialized("html_document::get_error_str");
    message = ft_strerror(FT_ERR_SUCCESS);
    if (message == ft_nullptr)
        message = "unknown error";
    return (message);
}

void html_document::clear() noexcept
{
    int lock_error;

    this->abort_if_not_initialized("html_document::clear");
    lock_error = pt_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    html_free_nodes(this->_root);
    this->_root = ft_nullptr;
    (void)pt_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *html_document::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("html_document::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
