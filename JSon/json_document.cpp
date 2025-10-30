#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

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

void json_document::set_error_unlocked(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void json_document::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int json_document::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

void json_document::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != ER_SUCCESS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void json_document::clear_unlocked() noexcept
{
    json_free_groups(this->_groups);
    this->_groups = ft_nullptr;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

char *json_document::write_to_string_unlocked() const noexcept
{
    char *result;

    result = json_write_to_string(this->_groups);
    if (!result)
    {
        int current_error;

        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    return (result);
}

json_document::json_document() noexcept
    : _groups(ft_nullptr)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

json_document::~json_document() noexcept
{
    this->clear();
    return ;
}

json_group *json_document::create_group(const char *name) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *group;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    group = json_create_json_group(name);
    if (!group)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (group);
}

json_item *json_document::create_item(const char *key, const char *value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!key || !value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

json_item *json_document::create_item(const char *key, const ft_big_number &value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

json_item *json_document::create_item(const char *key, const int value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

json_item *json_document::create_item(const char *key, const bool value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

void json_document::add_item(json_group *group, json_item *item) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_add_item_to_group(group, item);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::append_group(json_group *group) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_append_group(&this->_groups, group);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

int json_document::write_to_file(const char *file_path) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int result;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (!file_path)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    result = json_write_to_file(file_path, this->_groups);
    if (result != 0)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_HANDLE;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (result);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

int json_document::write_to_backend(ft_document_sink &sink) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    char *serialized_content;
    size_t serialized_length;
    int write_result;
    int sink_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    serialized_content = this->write_to_string_unlocked();
    if (!serialized_content)
    {
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    serialized_length = ft_strlen(serialized_content);
    write_result = sink.write_all(serialized_content, serialized_length);
    sink_error = sink.get_error();
    cma_free(serialized_content);
    if (write_result != ER_SUCCESS)
    {
        if (sink_error != ER_SUCCESS)
            write_result = sink_error;
        this->set_error_unlocked(write_result);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

char *json_document::write_to_string() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    char *result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    result = this->write_to_string_unlocked();
    json_document::restore_errno(guard, entry_errno);
    return (result);
}

int json_document::read_from_file(const char *file_path) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *groups;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (!file_path)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->clear_unlocked();
    groups = json_read_from_file(file_path);
    if (!groups)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_groups = groups;
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

int json_document::read_from_backend(ft_document_source &source) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *groups;
    int error_code;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->clear_unlocked();
    groups = json_read_from_backend(source);
    if (!groups)
    {
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = source.get_error();
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(error_code);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_groups = groups;
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

int json_document::read_from_file_streaming(const char *file_path, size_t buffer_capacity) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    FILE *file;
    json_group *groups;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (!file_path || buffer_capacity == 0)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    file = fopen(file_path, "rb");
    if (!file)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_IO);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->clear_unlocked();
    groups = json_read_from_file_stream(file, buffer_capacity);
    fclose(file);
    if (!groups)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_groups = groups;
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

int json_document::read_from_string(const char *content) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *groups;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (!content)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->clear_unlocked();
    groups = json_read_from_string(content);
    if (!groups)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_groups = groups;
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (0);
}

json_group *json_document::find_group(const char *name) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *group;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    group = json_find_group(this->_groups, name);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (group);
}

json_item *json_document::find_item(json_group *group, const char *key) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = json_find_item(group, key);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

json_item *json_document::find_item_by_pointer_unlocked(const char *pointer) const noexcept
{
    if (!pointer)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (pointer[0] == '\0' || pointer[0] != '/')
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    const char *cursor = pointer;
    json_group *current_group = ft_nullptr;
    bool expecting_group = true;
    while (*cursor)
    {
        if (*cursor != '/')
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        cursor += 1;
        const char *segment_start = cursor;
        while (*cursor && *cursor != '/')
            cursor += 1;
        size_t segment_length = static_cast<size_t>(cursor - segment_start);
        if (segment_length == 0)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        char *token = json_document_unescape_pointer_token(segment_start, segment_length);
        if (!token)
        {
            this->set_error_unlocked(ft_errno);
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
                this->set_error_unlocked(FT_ERR_NOT_FOUND);
                return (ft_nullptr);
            }
            current_group = group_iterator;
            expecting_group = false;
            if (*cursor == '\0')
            {
                this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
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
            this->set_error_unlocked(FT_ERR_NOT_FOUND);
            return (ft_nullptr);
        }
        if (*cursor != '\0')
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        this->set_error_unlocked(ER_SUCCESS);
        return (item_iterator);
    }
    this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
    return (ft_nullptr);
}

json_item *json_document::find_item_by_pointer(const char *pointer) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = this->find_item_by_pointer_unlocked(pointer);
    json_document::restore_errno(guard, entry_errno);
    return (item);
}

const char *json_document::get_value_by_pointer(const char *pointer) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    item = this->find_item_by_pointer_unlocked(pointer);
    if (!item)
    {
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (!item->value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (item->value);
}

void json_document::remove_group(const char *name) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_remove_group(&this->_groups, name);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::remove_item(json_group *group, const char *key) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_remove_item(group, key);
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const char *value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !key || !value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const int value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const bool value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::update_item(json_group *group, const char *key, const ft_big_number &value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_item *item;
    int current_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return ;
}

void json_document::clear() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return ;
    }
    this->clear_unlocked();
    json_document::restore_errno(guard, entry_errno);
    return ;
}

json_group *json_document::get_groups() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    json_group *groups;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    groups = this->_groups;
    this->set_error_unlocked(ER_SUCCESS);
    json_document::restore_errno(guard, entry_errno);
    return (groups);
}

void json_document::set_manual_error(int error_code) noexcept
{
    this->set_error(error_code);
    return ;
}

int json_document::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_value = this->_error_code;
    json_document::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *json_document::get_error_str() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    const char *error_string;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        json_document::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    json_document::restore_errno(guard, entry_errno);
    return (error_string);
}

void json_document::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

