#include "json.hpp"

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/move.hpp"

static void json_thread_error_push(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

static pt_mutex g_json_group_list_mutex;

static void json_group_initialize_error(json_group *group) noexcept
{
    if (group == ft_nullptr)
        return ;
    group->_error_code = FT_ERR_SUCCESS;
    return ;
}

static void json_item_initialize_error(json_item *item) noexcept
{
    if (item == ft_nullptr)
        return ;
    item->_error_code = FT_ERR_SUCCESS;
    return ;
}

static void json_schema_initialize_error(json_schema *schema) noexcept
{
    if (schema == ft_nullptr)
        return ;
    schema->_error_code = FT_ERR_SUCCESS;
    return ;
}

static void json_schema_field_initialize_error(json_schema_field *field) noexcept
{
    if (field == ft_nullptr)
        return ;
    field->_error_code = FT_ERR_SUCCESS;
    return ;
}

static void json_stream_reader_initialize_error(json_stream_reader *reader) noexcept
{
    if (reader == ft_nullptr)
        return ;
    reader->_error_code = FT_ERR_SUCCESS;
    return ;
}

int json_group_list_lock(ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;

    local_guard = ft_unique_lock<pt_mutex>(g_json_group_list_mutex);
    {
        int lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESS)
        {
            guard = ft_unique_lock<pt_mutex>();
            json_thread_error_push(lock_error);
            return (lock_error);
        }
    }
    guard = ft_move(local_guard);
    json_thread_error_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void json_group_list_finalize_lock(ft_unique_lock<pt_mutex> &guard)
{
    int current_error = FT_ERR_SUCCESS;

    if (guard.owns_lock())
        guard.unlock();
    int guard_error = ft_global_error_stack_drop_last_error();

    if (guard_error != FT_ERR_SUCCESS)
    {
        current_error = guard_error;
    }
    json_thread_error_push(current_error);
    return ;
}

void json_group_set_error_unlocked(json_group *group, int error_code)
{
    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    group->_error_code = error_code;
    json_thread_error_push(error_code);
    return ;
}

void json_group_set_error(json_group *group, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_group_lock(group, guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_group_set_error_unlocked(group, error_code);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int json_group_enable_thread_safety(json_group *group)
{
    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_group_initialize_error(group);
    json_group_set_error_unlocked(group, FT_ERR_SUCCESS);
    return (0);
}

int json_group_lock(json_group *group, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    local_guard = ft_unique_lock<pt_mutex>(group->_mutex);
    {
        int lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESS)
        {
            json_thread_error_push(lock_error);
            guard = ft_unique_lock<pt_mutex>();
            return (lock_error);
        }
    }
    guard = ft_move(local_guard);
    json_thread_error_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_group_get_error(const json_group *group)
{
    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    json_thread_error_push(FT_ERR_SUCCESS);
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
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    item->_error_code = error_code;
    json_thread_error_push(error_code);
    return ;
}

void json_item_set_error(json_item *item, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_item_lock(item, guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_item_set_error_unlocked(item, error_code);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int json_item_enable_thread_safety(json_item *item)
{
    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_item_initialize_error(item);
    json_item_set_error_unlocked(item, FT_ERR_SUCCESS);
    return (0);
}

int json_item_lock(json_item *item, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    int enable_error = json_item_enable_thread_safety(item);
    if (enable_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (enable_error);
    }
    local_guard = ft_unique_lock<pt_mutex>(item->_mutex);
    {
        int lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESS)
        {
            json_thread_error_push(lock_error);
            guard = ft_unique_lock<pt_mutex>();
            return (lock_error);
        }
    }
    guard = ft_move(local_guard);
    json_thread_error_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_item_get_error(const json_item *item)
{
    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    json_thread_error_push(FT_ERR_SUCCESS);
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
        return ;
    field->_error_code = error_code;
    return ;
}

void json_schema_field_set_error(json_schema_field *field, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (field == ft_nullptr)
        return ;
    lock_error = json_schema_field_lock(field, guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    json_schema_field_set_error_unlocked(field, error_code);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int json_schema_field_enable_thread_safety(json_schema_field *field)
{
    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    json_schema_field_initialize_error(field);
    json_schema_field_set_error_unlocked(field, FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_schema_field_lock(json_schema_field *field, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int enable_error;
    int mutex_error;

    if (field == ft_nullptr)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    enable_error = json_schema_field_enable_thread_safety(field);
    if (enable_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (enable_error);
    }
    local_guard = ft_unique_lock<pt_mutex>(field->_mutex);
    mutex_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (mutex_error);
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESS);
}

int json_schema_field_get_error(const json_schema_field *field)
{
    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
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
        return ;
    schema->_error_code = error_code;
    return ;
}

void json_schema_set_error(json_schema *schema, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (schema == ft_nullptr)
        return ;
    lock_error = json_schema_lock(schema, guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    json_schema_set_error_unlocked(schema, error_code);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int json_schema_enable_thread_safety(json_schema *schema)
{
    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    json_schema_initialize_error(schema);
    json_schema_set_error_unlocked(schema, FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_schema_lock(json_schema *schema, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;
    int enable_error;
    int mutex_error;

    if (schema == ft_nullptr)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    enable_error = json_schema_enable_thread_safety(schema);
    if (enable_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (enable_error);
    }
    local_guard = ft_unique_lock<pt_mutex>(schema->_mutex);
    mutex_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (mutex_error);
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESS);
}

int json_schema_get_error(const json_schema *schema)
{
    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
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
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    reader->_error_code = error_code;
    json_thread_error_push(error_code);
    return ;
}

void json_stream_reader_set_error(json_stream_reader *reader, int error_code)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_stream_reader_lock(reader, guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_stream_reader_set_error_unlocked(reader, error_code);
    json_stream_reader_finalize_lock(reader, guard);
    return ;
}

int json_stream_reader_enable_thread_safety(json_stream_reader *reader)
{
    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_stream_reader_initialize_error(reader);
    json_stream_reader_set_error_unlocked(reader, FT_ERR_SUCCESS);
    return (0);
}

int json_stream_reader_lock(json_stream_reader *reader, ft_unique_lock<pt_mutex> &guard)
{
    ft_unique_lock<pt_mutex> local_guard;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    int enable_error = json_stream_reader_enable_thread_safety(reader);
    if (enable_error != 0)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (enable_error);
    }
    local_guard = ft_unique_lock<pt_mutex>(reader->_mutex);
    {
        int lock_error = ft_global_error_stack_drop_last_error();

        if (lock_error != FT_ERR_SUCCESS)
        {
            json_thread_error_push(lock_error);
            guard = ft_unique_lock<pt_mutex>();
            return (lock_error);
        }
    }
    guard = ft_move(local_guard);
    json_thread_error_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void json_stream_reader_finalize_lock(json_stream_reader *reader,
        ft_unique_lock<pt_mutex> &guard)
{
    int current_errno;

    current_errno = ft_global_error_stack_peek_last_error();
    if (guard.owns_lock())
        guard.unlock();
    int guard_error = ft_global_error_stack_drop_last_error();

    if (guard_error != FT_ERR_SUCCESS)
    {
        json_stream_reader_set_error_unlocked(reader, guard_error);
        return ;
    }
    if (current_errno != FT_ERR_SUCCESS)
    {
        json_stream_reader_set_error_unlocked(reader, current_errno);
        return ;
    }
    json_stream_reader_set_error_unlocked(reader, FT_ERR_SUCCESS);
    return ;
}

int json_stream_reader_get_error(const json_stream_reader *reader)
{
    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    json_thread_error_push(FT_ERR_SUCCESS);
    return (reader->_error_code);
}

const char  *json_stream_reader_get_error_str(const json_stream_reader *reader)
{
    int error_code;

    error_code = json_stream_reader_get_error(reader);
    return (ft_strerror(error_code));
}
