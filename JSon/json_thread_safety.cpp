#include "json.hpp"

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

static thread_local int g_json_thread_error = FT_ERR_SUCCESS;
static pt_recursive_mutex *g_json_group_list_mutex = ft_nullptr;

static void json_thread_error_push(int error_code)
{
    g_json_thread_error = error_code;
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

int json_group_list_lock()
{
    int lock_error;

    if (g_json_group_list_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(g_json_group_list_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
}

int json_group_list_lock_manual()
{
    int lock_error;

    if (g_json_group_list_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(g_json_group_list_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
}

int json_group_list_unlock_manual()
{
    int unlock_error;

    if (g_json_group_list_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    unlock_error = pt_recursive_mutex_unlock_if_not_null(g_json_group_list_mutex);
    json_thread_error_push(unlock_error);
    return (unlock_error);
}

void json_group_list_finalize_lock()
{
    int unlock_error;

    if (g_json_group_list_mutex == ft_nullptr)
        return ;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(g_json_group_list_mutex);
    json_thread_error_push(unlock_error);
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
    int lock_error;
    int unlock_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (group->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return ;
    }
    lock_error = pt_mutex_lock_if_not_null(group->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_group_set_error_unlocked(group, error_code);
    unlock_error = pt_mutex_unlock_if_not_null(group->_mutex);
    json_thread_error_push(unlock_error);
    return ;
}

int json_group_enable_thread_safety(json_group *group)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (group->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
        {
            json_thread_error_push(FT_ERR_NO_MEMORY);
            return (FT_ERR_NO_MEMORY);
        }
        initialize_error = mutex_pointer->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            json_thread_error_push(initialize_error);
            return (initialize_error);
        }
        group->_mutex = mutex_pointer;
    }
    json_group_initialize_error(group);
    json_group_set_error_unlocked(group, FT_ERR_SUCCESS);
    return (0);
}

int json_group_disable_thread_safety(json_group *group)
{
    int destroy_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (group->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = group->_mutex->destroy();
    delete group->_mutex;
    group->_mutex = ft_nullptr;
    json_thread_error_push(destroy_error);
    return (destroy_error);
}

bool json_group_is_thread_safe(const json_group *group)
{
    if (group == ft_nullptr)
        return (false);
    return (group->_mutex != ft_nullptr);
}

int json_group_lock(json_group *group)
{
    int lock_error;

    if (group == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (group->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    lock_error = pt_mutex_lock_if_not_null(group->_mutex);
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
    int lock_error;
    int unlock_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (item->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return ;
    }
    lock_error = pt_mutex_lock_if_not_null(item->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_item_set_error_unlocked(item, error_code);
    unlock_error = pt_mutex_unlock_if_not_null(item->_mutex);
    json_thread_error_push(unlock_error);
    return ;
}

int json_item_enable_thread_safety(json_item *item)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (item->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
        {
            json_thread_error_push(FT_ERR_NO_MEMORY);
            return (FT_ERR_NO_MEMORY);
        }
        initialize_error = mutex_pointer->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            json_thread_error_push(initialize_error);
            return (initialize_error);
        }
        item->_mutex = mutex_pointer;
    }
    json_item_initialize_error(item);
    json_item_set_error_unlocked(item, FT_ERR_SUCCESS);
    return (0);
}

int json_item_disable_thread_safety(json_item *item)
{
    int destroy_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (item->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = item->_mutex->destroy();
    delete item->_mutex;
    item->_mutex = ft_nullptr;
    json_thread_error_push(destroy_error);
    return (destroy_error);
}

bool json_item_is_thread_safe(const json_item *item)
{
    if (item == ft_nullptr)
        return (false);
    return (item->_mutex != ft_nullptr);
}

int json_item_lock(json_item *item)
{
    int lock_error;

    if (item == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (item->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    lock_error = pt_mutex_lock_if_not_null(item->_mutex);
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
    int lock_error;
    int unlock_error;

    if (field == ft_nullptr)
        return ;
    if (field->_mutex == ft_nullptr)
        return ;
    lock_error = pt_mutex_lock_if_not_null(field->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    json_schema_field_set_error_unlocked(field, error_code);
    unlock_error = pt_mutex_unlock_if_not_null(field->_mutex);
    (void)unlock_error;
    return ;
}

int json_schema_field_enable_thread_safety(json_schema_field *field)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (field->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        initialize_error = mutex_pointer->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (initialize_error);
        }
        field->_mutex = mutex_pointer;
    }
    json_schema_field_initialize_error(field);
    json_schema_field_set_error_unlocked(field, FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_schema_field_disable_thread_safety(json_schema_field *field)
{
    int destroy_error;

    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (field->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = field->_mutex->destroy();
    delete field->_mutex;
    field->_mutex = ft_nullptr;
    return (destroy_error);
}

bool json_schema_field_is_thread_safe(const json_schema_field *field)
{
    if (field == ft_nullptr)
        return (false);
    return (field->_mutex != ft_nullptr);
}

int json_schema_field_lock(json_schema_field *field)
{
    int lock_error;

    if (field == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (field->_mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    lock_error = pt_mutex_lock_if_not_null(field->_mutex);
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
    int lock_error;
    int unlock_error;

    if (schema == ft_nullptr)
        return ;
    if (schema->_mutex == ft_nullptr)
        return ;
    lock_error = pt_mutex_lock_if_not_null(schema->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    json_schema_set_error_unlocked(schema, error_code);
    unlock_error = pt_mutex_unlock_if_not_null(schema->_mutex);
    (void)unlock_error;
    return ;
}

int json_schema_enable_thread_safety(json_schema *schema)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (schema->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        initialize_error = mutex_pointer->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (initialize_error);
        }
        schema->_mutex = mutex_pointer;
    }
    json_schema_initialize_error(schema);
    json_schema_set_error_unlocked(schema, FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int json_schema_disable_thread_safety(json_schema *schema)
{
    int destroy_error;

    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (schema->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = schema->_mutex->destroy();
    delete schema->_mutex;
    schema->_mutex = ft_nullptr;
    return (destroy_error);
}

bool json_schema_is_thread_safe(const json_schema *schema)
{
    if (schema == ft_nullptr)
        return (false);
    return (schema->_mutex != ft_nullptr);
}

int json_schema_lock(json_schema *schema)
{
    int lock_error;

    if (schema == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (schema->_mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    lock_error = pt_mutex_lock_if_not_null(schema->_mutex);
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
    int lock_error;
    int unlock_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (reader->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return ;
    }
    lock_error = pt_mutex_lock_if_not_null(reader->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(lock_error);
        return ;
    }
    json_stream_reader_set_error_unlocked(reader, error_code);
    unlock_error = pt_mutex_unlock_if_not_null(reader->_mutex);
    json_thread_error_push(unlock_error);
    return ;
}

int json_stream_reader_enable_thread_safety(json_stream_reader *reader)
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (reader->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
        {
            json_thread_error_push(FT_ERR_NO_MEMORY);
            return (FT_ERR_NO_MEMORY);
        }
        initialize_error = mutex_pointer->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            json_thread_error_push(initialize_error);
            return (initialize_error);
        }
        reader->_mutex = mutex_pointer;
    }
    json_stream_reader_initialize_error(reader);
    json_stream_reader_set_error_unlocked(reader, FT_ERR_SUCCESS);
    return (0);
}

int json_stream_reader_disable_thread_safety(json_stream_reader *reader)
{
    int destroy_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (reader->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = reader->_mutex->destroy();
    delete reader->_mutex;
    reader->_mutex = ft_nullptr;
    json_thread_error_push(destroy_error);
    return (destroy_error);
}

bool json_stream_reader_is_thread_safe(const json_stream_reader *reader)
{
    if (reader == ft_nullptr)
        return (false);
    return (reader->_mutex != ft_nullptr);
}

int json_stream_reader_lock(json_stream_reader *reader)
{
    int lock_error;

    if (reader == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (reader->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    lock_error = pt_mutex_lock_if_not_null(reader->_mutex);
    json_thread_error_push(lock_error);
    return (lock_error);
}

void json_stream_reader_finalize_lock(json_stream_reader *reader)
{
    int current_error;
    int unlock_error;

    current_error = g_json_thread_error;
    if (reader == ft_nullptr || reader->_mutex == ft_nullptr)
    {
        json_thread_error_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    unlock_error = pt_mutex_unlock_if_not_null(reader->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        json_thread_error_push(unlock_error);
        json_stream_reader_set_error_unlocked(reader, unlock_error);
        return ;
    }
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
int json_group_list_enable_thread_safety()
{
    if (g_json_group_list_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex = new (std::nothrow) pt_recursive_mutex();
    if (mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int initialize_error = mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex;
        return (initialize_error);
    }
    g_json_group_list_mutex = mutex;
    return (FT_ERR_SUCCESS);
}

int json_group_list_disable_thread_safety()
{
    if (g_json_group_list_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = g_json_group_list_mutex->destroy();
    delete g_json_group_list_mutex;
    g_json_group_list_mutex = ft_nullptr;
    return (destroy_error);
}
