#include "json.hpp"

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

static thread_local int g_json_thread_error = FT_ERR_SUCCESS;
static pt_mutex g_json_group_list_mutex;

static void json_thread_error_push(int error_code)
{
    g_json_thread_error = error_code;
    return ;
}

static int json_lock_guard(ft_unique_lock<pt_mutex> &guard, pt_mutex &mutex)
{
    int initialize_error;
    int lock_error;

    initialize_error = guard.initialize(mutex);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lock_error = guard.lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        guard.destroy();
        return (lock_error);
    }
    return (FT_ERR_SUCCESS);
}

static void json_unlock_guard(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        (void)guard.unlock();
    if (guard.is_initialized())
        (void)guard.destroy();
    return ;
}

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
    int lock_error = json_lock_guard(guard, g_json_group_list_mutex);

    json_thread_error_push(lock_error);
    return (lock_error);
}

void json_group_list_finalize_lock(ft_unique_lock<pt_mutex> &guard)
{
    json_unlock_guard(guard);
    json_thread_error_push(FT_ERR_SUCCESS);
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
    json_unlock_guard(guard);
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
    int lock_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_error = json_lock_guard(guard, group->_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
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
    return (ft_strerror(json_group_get_error(group)));
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
    json_unlock_guard(guard);
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
    int enable_error;
    int lock_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    enable_error = json_item_enable_thread_safety(item);
    if (enable_error != FT_ERR_SUCCESS)
        return (enable_error);
    lock_error = json_lock_guard(guard, item->_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
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
    return (ft_strerror(json_item_get_error(item)));
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
    json_unlock_guard(guard);
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
    int enable_error;
    int lock_error;

    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    enable_error = json_schema_field_enable_thread_safety(field);
    if (enable_error != FT_ERR_SUCCESS)
        return (enable_error);
    lock_error = json_lock_guard(guard, field->_mutex);
    return (lock_error);
}

int json_schema_field_get_error(const json_schema_field *field)
{
    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    return (field->_error_code);
}

const char *json_schema_field_get_error_str(const json_schema_field *field)
{
    return (ft_strerror(json_schema_field_get_error(field)));
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
    json_unlock_guard(guard);
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
    int enable_error;
    int lock_error;

    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    enable_error = json_schema_enable_thread_safety(schema);
    if (enable_error != FT_ERR_SUCCESS)
        return (enable_error);
    lock_error = json_lock_guard(guard, schema->_mutex);
    return (lock_error);
}

int json_schema_get_error(const json_schema *schema)
{
    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    return (schema->_error_code);
}

const char *json_schema_get_error_str(const json_schema *schema)
{
    return (ft_strerror(json_schema_get_error(schema)));
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
    int enable_error;
    int lock_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    enable_error = json_stream_reader_enable_thread_safety(reader);
    if (enable_error != 0)
        return (enable_error);
    lock_error = json_lock_guard(guard, reader->_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
}

void json_stream_reader_finalize_lock(json_stream_reader *reader,
    ft_unique_lock<pt_mutex> &guard)
{
    int current_error;

    current_error = g_json_thread_error;
    json_unlock_guard(guard);
    if (current_error != FT_ERR_SUCCESS)
    {
        json_stream_reader_set_error_unlocked(reader, current_error);
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

const char *json_stream_reader_get_error_str(const json_stream_reader *reader)
{
    return (ft_strerror(json_stream_reader_get_error(reader)));
}
