#include "json.hpp"

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/move.hpp"

static pt_mutex g_json_group_list_mutex;

static void json_group_initialize_error(json_group *group) noexcept
{
    if (group == ft_nullptr)
        return ;
    group->_error_code = FT_ER_SUCCESSS;
    return ;
}

static void json_item_initialize_error(json_item *item) noexcept
{
    if (item == ft_nullptr)
        return ;
    item->_error_code = FT_ER_SUCCESSS;
    return ;
}

static void json_schema_initialize_error(json_schema *schema) noexcept
{
    if (schema == ft_nullptr)
        return ;
    schema->_error_code = FT_ER_SUCCESSS;
    return ;
}

static void json_schema_field_initialize_error(json_schema_field *field) noexcept
{
    if (field == ft_nullptr)
        return ;
    field->_error_code = FT_ER_SUCCESSS;
    return ;
}

static void json_stream_reader_initialize_error(json_stream_reader *reader) noexcept
{
    if (reader == ft_nullptr)
        return ;
    reader->_error_code = FT_ER_SUCCESSS;
    return ;
}

int json_group_list_lock(ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;

    local_guard = ft_unique_lock<pt_mutex>(g_json_group_list_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_group_list_restore(ft_unique_lock<pt_mutex> &guard, int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        ft_errno = current_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void json_group_set_error_unlocked(json_group *group, int error_code)
{
    if (group == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    group->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void json_group_set_error(json_group *group, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (group == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    entry_errno = ft_errno;
    lock_error = json_group_lock(group, guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        ft_errno = lock_error;
        return ;
    }
    json_group_set_error_unlocked(group, error_code);
    json_group_restore_errno(group, guard, entry_errno);
    return ;
}

int json_group_enable_thread_safety(json_group *group)
{
    if (group == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_group_initialize_error(group);
    json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
    return (0);
}

int json_group_lock(json_group *group, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int entry_errno;

    if (group == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry_errno = ft_errno;
    local_guard = ft_unique_lock<pt_mutex>(group->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_group_restore_errno(json_group *group, ft_unique_lock<pt_mutex> &guard, int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        json_group_set_error_unlocked(group, guard.get_error());
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        json_group_set_error_unlocked(group, current_errno);
        return ;
    }
    json_group_set_error_unlocked(group, entry_errno);
    return ;
}

int json_group_get_error(const json_group *group)
{
    if (group == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (group->_error_code);
}

const char *json_group_get_error_str(const json_group *group)
{
    int error_code;

    error_code = json_group_get_error(group);
    return (ft_strerror(error_code));
}

void json_item_set_error_unlocked(json_item *item, int error_code)
{
    if (item == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    item->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void json_item_set_error(json_item *item, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (item == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    entry_errno = ft_errno;
    lock_error = json_item_lock(item, guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        ft_errno = lock_error;
        return ;
    }
    json_item_set_error_unlocked(item, error_code);
    json_item_restore_errno(item, guard, entry_errno);
    return ;
}

int json_item_enable_thread_safety(json_item *item)
{
    if (item == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_item_initialize_error(item);
    json_item_set_error_unlocked(item, FT_ER_SUCCESSS);
    return (0);
}

int json_item_lock(json_item *item, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int entry_errno;

    if (item == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (json_item_enable_thread_safety(item) != 0)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (ft_errno);
    }
    entry_errno = ft_errno;
    local_guard = ft_unique_lock<pt_mutex>(item->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_item_restore_errno(json_item *item, ft_unique_lock<pt_mutex> &guard, int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        json_item_set_error_unlocked(item, guard.get_error());
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        json_item_set_error_unlocked(item, current_errno);
        return ;
    }
    json_item_set_error_unlocked(item, entry_errno);
    return ;
}

int json_item_get_error(const json_item *item)
{
    if (item == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (item->_error_code);
}

const char *json_item_get_error_str(const json_item *item)
{
    int error_code;

    error_code = json_item_get_error(item);
    return (ft_strerror(error_code));
}

void json_schema_field_set_error_unlocked(json_schema_field *field, int error_code)
{
    if (field == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    field->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void json_schema_field_set_error(json_schema_field *field, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (field == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    entry_errno = ft_errno;
    lock_error = json_schema_field_lock(field, guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        ft_errno = lock_error;
        return ;
    }
    json_schema_field_set_error_unlocked(field, error_code);
    json_schema_field_restore_errno(field, guard, entry_errno);
    return ;
}

int json_schema_field_enable_thread_safety(json_schema_field *field)
{
    if (field == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_schema_field_initialize_error(field);
    json_schema_field_set_error_unlocked(field, FT_ER_SUCCESSS);
    return (0);
}

int json_schema_field_lock(json_schema_field *field, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int entry_errno;

    if (field == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (json_schema_field_enable_thread_safety(field) != 0)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (ft_errno);
    }
    entry_errno = ft_errno;
    local_guard = ft_unique_lock<pt_mutex>(field->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_schema_field_restore_errno(json_schema_field *field, ft_unique_lock<pt_mutex> &guard, int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        json_schema_field_set_error_unlocked(field, guard.get_error());
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        json_schema_field_set_error_unlocked(field, current_errno);
        return ;
    }
    json_schema_field_set_error_unlocked(field, entry_errno);
    return ;
}

int json_schema_field_get_error(const json_schema_field *field)
{
    if (field == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (field->_error_code);
}

const char *json_schema_field_get_error_str(const json_schema_field *field)
{
    int error_code;

    error_code = json_schema_field_get_error(field);
    return (ft_strerror(error_code));
}

void json_schema_set_error_unlocked(json_schema *schema, int error_code)
{
    if (schema == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    schema->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void json_schema_set_error(json_schema *schema, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (schema == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    entry_errno = ft_errno;
    lock_error = json_schema_lock(schema, guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        ft_errno = lock_error;
        return ;
    }
    json_schema_set_error_unlocked(schema, error_code);
    json_schema_restore_errno(schema, guard, entry_errno);
    return ;
}

int json_schema_enable_thread_safety(json_schema *schema)
{
    if (schema == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_schema_initialize_error(schema);
    json_schema_set_error_unlocked(schema, FT_ER_SUCCESSS);
    return (0);
}

int json_schema_lock(json_schema *schema, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int entry_errno;

    if (schema == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (json_schema_enable_thread_safety(schema) != 0)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (ft_errno);
    }
    entry_errno = ft_errno;
    local_guard = ft_unique_lock<pt_mutex>(schema->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_schema_restore_errno(json_schema *schema, ft_unique_lock<pt_mutex> &guard, int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        json_schema_set_error_unlocked(schema, guard.get_error());
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        json_schema_set_error_unlocked(schema, current_errno);
        return ;
    }
    json_schema_set_error_unlocked(schema, entry_errno);
    return ;
}

int json_schema_get_error(const json_schema *schema)
{
    if (schema == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (schema->_error_code);
}

const char *json_schema_get_error_str(const json_schema *schema)
{
    int error_code;

    error_code = json_schema_get_error(schema);
    return (ft_strerror(error_code));
}

void json_stream_reader_set_error_unlocked(json_stream_reader *reader, int error_code)
{
    if (reader == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    reader->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void json_stream_reader_set_error(json_stream_reader *reader, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (reader == ft_nullptr)
    {
        ft_errno = error_code;
        return ;
    }
    entry_errno = ft_errno;
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        return ;
    }
    json_stream_reader_set_error_unlocked(reader, error_code);
    json_stream_reader_restore_errno(reader, guard, entry_errno);
    return ;
}

int json_stream_reader_enable_thread_safety(json_stream_reader *reader)
{
    if (reader == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    json_stream_reader_initialize_error(reader);
    json_stream_reader_set_error_unlocked(reader, FT_ER_SUCCESSS);
    return (0);
}

int json_stream_reader_lock(json_stream_reader *reader, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int entry_errno;

    if (reader == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (json_stream_reader_enable_thread_safety(reader) != 0)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (ft_errno);
    }
    entry_errno = ft_errno;
    local_guard = ft_unique_lock<pt_mutex>(reader->_mutex);
    if (local_guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (FT_ER_SUCCESSS);
}

void json_stream_reader_restore_errno(json_stream_reader *reader,
        ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        json_stream_reader_set_error_unlocked(reader, guard.get_error());
        return ;
    }
    if (current_errno != FT_ER_SUCCESSS)
    {
        json_stream_reader_set_error_unlocked(reader, current_errno);
        return ;
    }
    json_stream_reader_set_error_unlocked(reader, entry_errno);
    return ;
}

int json_stream_reader_get_error(const json_stream_reader *reader)
{
    if (reader == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (reader->_error_code);
}

const char  *json_stream_reader_get_error_str(const json_stream_reader *reader)
{
    int error_code;

    error_code = json_stream_reader_get_error(reader);
    return (ft_strerror(error_code));
}
