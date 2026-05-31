#include "json.hpp"
#include "json_schema.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/pthread_internal.hpp"

static ft_bool json_is_number(const char *string)
{
    if (!string || !(*string))
        return (FT_FALSE);
    char *end_pointer = ft_nullptr;
    ft_strtol(string, &end_pointer, 10);
    if (end_pointer == string)
        return (FT_FALSE);
    if (end_pointer && *end_pointer == '\0')
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_bool json_is_bool(const char *string)
{
    if (ft_strcmp(string, "true") == FT_ERR_SUCCESS)
        return (FT_TRUE);
    if (ft_strcmp(string, "false") == FT_ERR_SUCCESS)
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool json_validate_schema(json_group *group, const json_schema &schema)
{
    json_schema *schema_mutable;
    json_schema_field *field;
    ft_bool validation_result;
    int32_t validation_error;
    int32_t lock_error;
    int32_t field_lock_error;

    if (!group)
        return (FT_FALSE);
    schema_mutable = const_cast<json_schema *>(&schema);
    validation_error = json_schema_enable_thread_safety(schema_mutable);
    if (validation_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (schema_mutable->_mutex == ft_nullptr)
        return (FT_FALSE);
    lock_error = pt_recursive_mutex_lock_if_not_null(schema_mutable->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    field = schema_mutable->fields;
    validation_result = FT_TRUE;
    validation_error = FT_ERR_SUCCESS;
    while (validation_result == FT_TRUE && field)
    {
        json_schema_field *next_field;
        int32_t field_error;

        field_error = json_schema_field_enable_thread_safety(field);
        if (field_error != FT_ERR_SUCCESS)
        {
            json_schema_set_error_unlocked(schema_mutable, field_error);
            validation_error = field_error;
            validation_result = FT_FALSE;
            break ;
        }
        if (field->_mutex == ft_nullptr)
        {
            json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_STATE);
            validation_error = FT_ERR_INVALID_STATE;
            validation_result = FT_FALSE;
            break ;
        }
        field_lock_error = pt_recursive_mutex_lock_if_not_null(field->_mutex);
        if (field_lock_error != FT_ERR_SUCCESS)
        {
            json_schema_set_error_unlocked(schema_mutable, field_lock_error);
            validation_error = field_lock_error;
            validation_result = FT_FALSE;
            break ;
        }
        next_field = field->next;
        if (field->key == ft_nullptr)
        {
            json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
            json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
            validation_error = FT_ERR_INVALID_ARGUMENT;
            validation_result = FT_FALSE;
        }
        else
        {
            json_item *item;
            ft_bool type_invalid;

            item = json_find_item(group, field->key);
            if (!item)
            {
                json_schema_field_set_error_unlocked(field, FT_ERR_NOT_FOUND);
                if (field->required == FT_TRUE)
                {
                    json_schema_set_error_unlocked(schema_mutable, FT_ERR_NOT_FOUND);
                    validation_error = FT_ERR_NOT_FOUND;
                    validation_result = FT_FALSE;
                }
            }
            else
            {
                type_invalid = FT_FALSE;
                if (field->type == JSON_NUMBER)
                {
                    if (json_is_number(item->value) == FT_FALSE)
                        type_invalid = FT_TRUE;
                }
                else if (field->type == JSON_BOOL)
                {
                    if (json_is_bool(item->value) == FT_FALSE)
                        type_invalid = FT_TRUE;
                }
                if (type_invalid == FT_TRUE)
                {
                    json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
                    json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
                    validation_error = FT_ERR_INVALID_ARGUMENT;
                    validation_result = FT_FALSE;
                }
                else
                    json_schema_field_set_error_unlocked(field, FT_ERR_SUCCESS);
            }
        }
        if (validation_result == FT_FALSE)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(field->_mutex);
            break ;
        }
        (void)pt_recursive_mutex_unlock_if_not_null(field->_mutex);
        field = next_field;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(schema_mutable->_mutex);
    if (validation_result == FT_TRUE)
    {
        json_schema_set_error_unlocked(schema_mutable, FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    if (validation_error == FT_ERR_SUCCESS)
        validation_error = schema_mutable->_error_code;
    return (FT_FALSE);
}
