#include "json_document.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"

json_document::json_document() noexcept
    : _groups(ft_nullptr)
{
    return ;
}

json_document::~json_document() noexcept
{
    this->clear();
    return ;
}

json_group *json_document::create_group(const char *name) noexcept
{
    return (json_create_json_group(name));
}

json_item *json_document::create_item(const char *key, const char *value) noexcept
{
    return (json_create_item(key, value));
}

json_item *json_document::create_item(const char *key, const int value) noexcept
{
    return (json_create_item(key, value));
}

json_item *json_document::create_item(const char *key, const bool value) noexcept
{
    return (json_create_item(key, value));
}

void json_document::add_item(json_group *group, json_item *item) noexcept
{
    json_add_item_to_group(group, item);
    return ;
}

void json_document::append_group(json_group *group) noexcept
{
    json_append_group(&this->_groups, group);
    return ;
}

int json_document::write_to_file(const char *file_path) const noexcept
{
    return (json_write_to_file(file_path, this->_groups));
}

char *json_document::write_to_string() const noexcept
{
    return (json_write_to_string(this->_groups));
}

int json_document::read_from_file(const char *file_path) noexcept
{
    this->clear();
    this->_groups = json_read_from_file(file_path);
    if (!this->_groups)
        return (-1);
    return (0);
}

int json_document::read_from_string(const char *content) noexcept
{
    this->clear();
    this->_groups = json_read_from_string(content);
    if (!this->_groups)
        return (-1);
    return (0);
}

json_group *json_document::find_group(const char *name) const noexcept
{
    return (json_find_group(this->_groups, name));
}

json_item *json_document::find_item(json_group *group, const char *key) const noexcept
{
    return (json_find_item(group, key));
}

void json_document::remove_group(const char *name) noexcept
{
    json_remove_group(&this->_groups, name);
    return ;
}

void json_document::remove_item(json_group *group, const char *key) noexcept
{
    json_remove_item(group, key);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const char *value) noexcept
{
    json_update_item(group, key, value);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const int value) noexcept
{
    json_update_item(group, key, value);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const bool value) noexcept
{
    json_update_item(group, key, value);
    return ;
}

void json_document::clear() noexcept
{
    json_free_groups(this->_groups);
    this->_groups = ft_nullptr;
    return ;
}

