#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#include <cstdio>
#include <cstddef>

#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"

static char *json_document_unescape_pointer_token(const char *start, size_t length) noexcept
{
    if (!start && length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    size_t allocation_size = length + 1;
    char *token = static_cast<char *>(cma_malloc(allocation_size));
    if (!token)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    size_t input_index = 0;
    size_t output_index = 0;
    while (input_index < length)
    {
        char current_character = start[input_index];
        if (current_character == '~')
        {
            if (input_index + 1 >= length)
            {
                cma_free(token);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (ft_nullptr);
            }
            char escape_character = start[input_index + 1];
            if (escape_character == '0')
                token[output_index] = '~';
            else if (escape_character == '1')
                token[output_index] = '/';
            else
            {
                cma_free(token);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (ft_nullptr);
            }
            input_index += 2;
            output_index += 1;
            continue;
        }
        token[output_index] = current_character;
        output_index += 1;
        input_index += 1;
    }
    token[output_index] = '\0';
    ft_errno = ER_SUCCESS;
    return (token);
}

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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_group *group = json_create_json_group(name);
    if (!group)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_item *item = json_create_item(key, value);
    if (!item)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int result = json_write_to_file(file_path, this->_groups);
    if (result != 0)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_HANDLE;
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
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    this->clear();
    json_group *groups = json_read_from_file(file_path);
    if (!groups)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error(current_error);
        return (-1);
    }
    this->_groups = groups;
    this->set_error(ER_SUCCESS);
    return (0);
}

int json_document::read_from_file_streaming(const char *file_path, size_t buffer_capacity) noexcept
{
    if (!file_path || buffer_capacity == 0)
    {
        this->clear();
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        this->clear();
        this->set_error(FT_ERR_IO);
        return (-1);
    }
    this->clear();
    json_group *groups = json_read_from_file_stream(file, buffer_capacity);
    fclose(file);
    if (!groups)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    this->clear();
    json_group *groups = json_read_from_string(content);
    if (!groups)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    json_item *item = json_find_item(group, key);
    this->set_error(ER_SUCCESS);
    return (item);
}

json_item *json_document::find_item_by_pointer(const char *pointer) const noexcept
{
    if (!pointer)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (pointer[0] == '\0' || pointer[0] != '/')
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    const char *cursor = pointer;
    json_group *current_group = ft_nullptr;
    bool expecting_group = true;
    while (*cursor)
    {
        if (*cursor != '/')
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        cursor += 1;
        const char *segment_start = cursor;
        while (*cursor && *cursor != '/')
            cursor += 1;
        size_t segment_length = static_cast<size_t>(cursor - segment_start);
        if (segment_length == 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        char *token = json_document_unescape_pointer_token(segment_start, segment_length);
        if (!token)
        {
            this->set_error(ft_errno);
            return (ft_nullptr);
        }
        if (expecting_group)
        {
            json_group *group_iterator = this->_groups;
            bool found_group = false;
            while (group_iterator)
            {
                const char *group_name = group_iterator->name;
                if (!group_name)
                    group_name = "";
                if (ft_strcmp(group_name, token) == 0)
                {
                    found_group = true;
                    break;
                }
                group_iterator = group_iterator->next;
            }
            cma_free(token);
            if (!found_group)
            {
                this->set_error(FT_ERR_NOT_FOUND);
                return (ft_nullptr);
            }
            current_group = group_iterator;
            expecting_group = false;
            if (*cursor == '\0')
            {
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            continue;
        }
        json_item *item_iterator = ft_nullptr;
        if (current_group)
            item_iterator = current_group->items;
        while (item_iterator)
        {
            const char *item_key = item_iterator->key;
            if (!item_key)
                item_key = "";
            if (ft_strcmp(item_key, token) == 0)
                break;
            item_iterator = item_iterator->next;
        }
        cma_free(token);
        if (!item_iterator)
        {
            this->set_error(FT_ERR_NOT_FOUND);
            return (ft_nullptr);
        }
        if (*cursor != '\0')
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        this->set_error(ER_SUCCESS);
        return (item_iterator);
    }
    this->set_error(FT_ERR_INVALID_ARGUMENT);
    return (ft_nullptr);
}

const char *json_document::get_value_by_pointer(const char *pointer) const noexcept
{
    json_item *item = this->find_item_by_pointer(pointer);
    if (!item)
        return (ft_nullptr);
    if (!item->value)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (item->value);
}

void json_document::remove_group(const char *name) noexcept
{
    if (!name)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        int current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
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

