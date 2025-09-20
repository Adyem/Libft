#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

json_document::json_document() noexcept
    : _groups(ft_nullptr), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

json_document::~json_document() noexcept
{
    this->clear();
    return ;
}

json_group *json_document::create_group(const char *name) noexcept
{
    if (!name)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_group *group = json_create_json_group(name);
    if (!group)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (group);
}

json_item *json_document::create_item(const char *key, const char *value) noexcept
{
    if (!key || !value)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (item);
}

json_item *json_document::create_item(const char *key, const ft_big_number &value) noexcept
{
    if (!key)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (item);
}

json_item *json_document::create_item(const char *key, const int value) noexcept
{
    if (!key)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (item);
}

json_item *json_document::create_item(const char *key, const bool value) noexcept
{
    if (!key)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (item);
}

void json_document::add_item(json_group *group, json_item *item) noexcept
{
    if (!group || !item)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_add_item_to_group(group, item);
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::append_group(json_group *group) noexcept
{
    if (!group)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_append_group(&this->_groups, group);
    this->set_error(ER_SUCCESS);
    return ;
}

int json_document::write_to_file(const char *file_path) const noexcept
{
    if (!file_path)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    int result = json_write_to_file(file_path, this->_groups);
    if (result != 0)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FILE_INVALID_FD;
        this->set_error(current_error);
        return (result);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

char *json_document::write_to_string() const noexcept
{
    char *result = json_write_to_string(this->_groups);
    if (!result)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (result);
}

int json_document::read_from_file(const char *file_path) noexcept
{
    if (!file_path)
    {
        this->clear();
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->clear();
    json_group *groups = json_read_from_file(file_path);
    if (!groups)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_EINVAL;
        this->set_error(current_error);
        return (-1);
    }
    this->_groups = groups;
    this->set_error(ER_SUCCESS);
    return (0);
}

int json_document::read_from_string(const char *content) noexcept
{
    if (!content)
    {
        this->clear();
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->clear();
    json_group *groups = json_read_from_string(content);
    if (!groups)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_EINVAL;
        this->set_error(current_error);
        return (-1);
    }
    this->_groups = groups;
    this->set_error(ER_SUCCESS);
    return (0);
}

json_group *json_document::find_group(const char *name) const noexcept
{
    if (!name)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_group *group = json_find_group(this->_groups, name);
    this->set_error(ER_SUCCESS);
    return (group);
}

json_item *json_document::find_item(json_group *group, const char *key) const noexcept
{
    if (!group || !key)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    json_item *item = json_find_item(group, key);
    this->set_error(ER_SUCCESS);
    return (item);
}

void json_document::remove_group(const char *name) noexcept
{
    if (!name)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_remove_group(&this->_groups, name);
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::remove_item(json_group *group, const char *key) noexcept
{
    if (!group || !key)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_remove_item(group, key);
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const char *value) noexcept
{
    if (!group || !key || !value)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const int value) noexcept
{
    if (!group || !key)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const bool value) noexcept
{
    if (!group || !key)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const ft_big_number &value) noexcept
{
    if (!group || !key)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = JSON_MALLOC_FAIL;
        this->set_error(current_error);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void json_document::clear() noexcept
{
    json_free_groups(this->_groups);
    this->_groups = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

int json_document::get_error() const noexcept
{
    return (this->_error_code);
}

const char *json_document::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void json_document::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

