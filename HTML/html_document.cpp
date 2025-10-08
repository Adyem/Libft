#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

html_document::html_document() noexcept
    : _root(ft_nullptr), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

html_document::~html_document() noexcept
{
    this->clear();
    return ;
}

html_node *html_document::create_node(const char *tag_name, const char *text_content) noexcept
{
    html_node *node;

    if (!tag_name)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    node = html_create_node(tag_name, text_content);
    if (!node)
    {
        if (ft_errno == ER_SUCCESS)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->set_error(ft_errno);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (node);
}

html_attr *html_document::create_attr(const char *key, const char *value) noexcept
{
    html_attr *attribute;

    if (!key || !value)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    attribute = html_create_attr(key, value);
    if (!attribute)
    {
        if (ft_errno == ER_SUCCESS)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->set_error(ft_errno);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (attribute);
}

void html_document::add_attr(html_node *target_node, html_attr *new_attribute) noexcept
{
    if (!target_node || !new_attribute)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_add_attr(target_node, new_attribute);
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::remove_attr(html_node *target_node, const char *key) noexcept
{
    if (!target_node || !key)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_attr(target_node, key);
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::add_child(html_node *parent_node, html_node *child_node) noexcept
{
    if (!parent_node || !child_node)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_add_child(parent_node, child_node);
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::append_node(html_node *new_node) noexcept
{
    if (!new_node)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_append_node(&this->_root, new_node);
    this->set_error(ER_SUCCESS);
    return ;
}

int html_document::write_to_file(const char *file_path) const noexcept
{
    int result;

    if (!file_path)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    result = html_write_to_file(file_path, this->_root);
    if (result != 0)
    {
        if (ft_errno == ER_SUCCESS)
            this->set_error(FT_ERR_INVALID_HANDLE);
        else
            this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

char *html_document::write_to_string() const noexcept
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = html_write_to_string(this->_root);
    if (!result)
    {
        if (ft_errno == ER_SUCCESS)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->set_error(ft_errno);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (result);
}

void html_document::remove_nodes_by_tag(const char *tag_name) noexcept
{
    if (!tag_name)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_tag(&this->_root, tag_name);
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::remove_nodes_by_attr(const char *key, const char *value) noexcept
{
    if (!key || !value)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_attr(&this->_root, key, value);
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::remove_nodes_by_text(const char *text_content) noexcept
{
    if (!text_content)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    html_remove_nodes_by_text(&this->_root, text_content);
    this->set_error(ER_SUCCESS);
    return ;
}

html_node *html_document::find_by_tag(const char *tag_name) const noexcept
{
    html_node *node;

    if (!tag_name)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_tag(this->_root, tag_name);
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (node);
}

html_node *html_document::find_by_attr(const char *key, const char *value) const noexcept
{
    html_node *node;

    if (!key || !value)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_attr(this->_root, key, value);
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (node);
}

html_node *html_document::find_by_text(const char *text_content) const noexcept
{
    html_node *node;

    if (!text_content)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_text(this->_root, text_content);
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (node);
}

html_node *html_document::find_by_selector(const char *selector) const noexcept
{
    html_node *node;

    if (!selector)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    node = html_find_by_selector(this->_root, selector);
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (node);
}

size_t html_document::count_nodes_by_tag(const char *tag_name) const noexcept
{
    size_t count;

    if (!tag_name)
    {
        const_cast<html_document *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    count = html_count_nodes_by_tag(this->_root, tag_name);
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (count);
}

html_node *html_document::get_root() const noexcept
{
    const_cast<html_document *>(this)->set_error(ER_SUCCESS);
    return (this->_root);
}

int html_document::get_error() const noexcept
{
    return (this->_error_code);
}

const char *html_document::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void html_document::clear() noexcept
{
    html_free_nodes(this->_root);
    this->_root = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

void html_document::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

