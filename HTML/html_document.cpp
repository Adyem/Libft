#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"

html_document::html_document() noexcept
    : _root(ft_nullptr)
{
    return ;
}

html_document::~html_document() noexcept
{
    this->clear();
    return ;
}

html_node *html_document::create_node(const char *tag_name, const char *text_content) noexcept
{
    return (html_create_node(tag_name, text_content));
}

html_attr *html_document::create_attr(const char *key, const char *value) noexcept
{
    return (html_create_attr(key, value));
}

void html_document::add_attr(html_node *target_node, html_attr *new_attribute) noexcept
{
    html_add_attr(target_node, new_attribute);
    return ;
}

void html_document::remove_attr(html_node *target_node, const char *key) noexcept
{
    html_remove_attr(target_node, key);
    return ;
}

void html_document::add_child(html_node *parent_node, html_node *child_node) noexcept
{
    html_add_child(parent_node, child_node);
    return ;
}

void html_document::append_node(html_node *new_node) noexcept
{
    html_append_node(&this->_root, new_node);
    return ;
}

int html_document::write_to_file(const char *file_path) const noexcept
{
    return (html_write_to_file(file_path, this->_root));
}

char *html_document::write_to_string() const noexcept
{
    return (html_write_to_string(this->_root));
}

void html_document::remove_nodes_by_tag(const char *tag_name) noexcept
{
    html_remove_nodes_by_tag(&this->_root, tag_name);
    return ;
}

void html_document::remove_nodes_by_attr(const char *key, const char *value) noexcept
{
    html_remove_nodes_by_attr(&this->_root, key, value);
    return ;
}

void html_document::remove_nodes_by_text(const char *text_content) noexcept
{
    html_remove_nodes_by_text(&this->_root, text_content);
    return ;
}

html_node *html_document::find_by_tag(const char *tag_name) const noexcept
{
    return (html_find_by_tag(this->_root, tag_name));
}

html_node *html_document::find_by_attr(const char *key, const char *value) const noexcept
{
    return (html_find_by_attr(this->_root, key, value));
}

html_node *html_document::find_by_text(const char *text_content) const noexcept
{
    return (html_find_by_text(this->_root, text_content));
}

size_t html_document::count_nodes_by_tag(const char *tag_name) const noexcept
{
    return (html_count_nodes_by_tag(this->_root, tag_name));
}

void html_document::clear() noexcept
{
    html_free_nodes(this->_root);
    this->_root = ft_nullptr;
    return ;
}

