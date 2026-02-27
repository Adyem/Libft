#include "json.hpp"
#include "json_schema.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread_internal.hpp"

static bool json_is_number(const char *string)
{
    if (!string || !(*string))
        return (false);
    char *end_pointer = ft_nullptr;
    ft_strtol(string, &end_pointer, 10);
    if (end_pointer == string)
        return (false);
    if (end_pointer && *end_pointer == '\0')
        return (true);
    return (false);
}

static bool json_is_bool(const char *string)
{
    if (ft_strcmp(string, "true") == 0)
        return (true);
    if (ft_strcmp(string, "false") == 0)
        return (true);
    return (false);
}

bool json_validate_schema(json_group *group, const json_schema &schema)
{
    json_schema *schema_mutable;
    json_schema_field *field;
    bool validation_result;
    int validation_error;
    int lock_error;
    int unlock_error;
    int field_lock_error;
    int field_unlock_error;

    if (!group)
        return (false);
    schema_mutable = const_cast<json_schema *>(&schema);
    validation_error = json_schema_enable_thread_safety(schema_mutable);
    if (validation_error != FT_ERR_SUCCESS)
        return (false);
    if (schema_mutable->_mutex == ft_nullptr)
        return (false);
    lock_error = pt_mutex_lock_if_not_null(schema_mutable->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    field = schema_mutable->fields;
    validation_result = true;
    validation_error = FT_ERR_SUCCESS;
    while (validation_result == true && field)
    {
        json_schema_field *next_field;
        int field_error;

        field_error = json_schema_field_enable_thread_safety(field);
        if (field_error != FT_ERR_SUCCESS)
        {
            json_schema_set_error_unlocked(schema_mutable, field_error);
            validation_error = field_error;
            validation_result = false;
            break ;
        }
        if (field->_mutex == ft_nullptr)
        {
            json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_STATE);
            validation_error = FT_ERR_INVALID_STATE;
            validation_result = false;
            break ;
        }
        field_lock_error = pt_mutex_lock_if_not_null(field->_mutex);
        if (field_lock_error != FT_ERR_SUCCESS)
        {
            json_schema_set_error_unlocked(schema_mutable, field_lock_error);
            validation_error = field_lock_error;
            validation_result = false;
            break ;
        }
        next_field = field->next;
        if (field->key == ft_nullptr)
        {
            json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
            json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
            validation_error = FT_ERR_INVALID_ARGUMENT;
            validation_result = false;
        }
        else
        {
            json_item *item;
            bool type_invalid;

            item = json_find_item(group, field->key);
            if (!item)
            {
                json_schema_field_set_error_unlocked(field, FT_ERR_NOT_FOUND);
                if (field->required == true)
                {
                    json_schema_set_error_unlocked(schema_mutable, FT_ERR_NOT_FOUND);
                    validation_error = FT_ERR_NOT_FOUND;
                    validation_result = false;
                }
            }
            else
            {
                type_invalid = false;
                if (field->type == JSON_NUMBER)
                {
                    if (json_is_number(item->value) == false)
                        type_invalid = true;
                }
                else if (field->type == JSON_BOOL)
                {
                    if (json_is_bool(item->value) == false)
                        type_invalid = true;
                }
                if (type_invalid == true)
                {
                    json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
                    json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
                    validation_error = FT_ERR_INVALID_ARGUMENT;
                    validation_result = false;
                }
                else
                    json_schema_field_set_error_unlocked(field, FT_ERR_SUCCESS);
            }
        }
        if (validation_result == false)
        {
            field_unlock_error = pt_mutex_unlock_if_not_null(field->_mutex);
            (void)field_unlock_error;
            break ;
        }
        field_unlock_error = pt_mutex_unlock_if_not_null(field->_mutex);
        if (field_unlock_error != FT_ERR_SUCCESS)
        {
            json_schema_set_error_unlocked(schema_mutable, field_unlock_error);
            validation_error = field_unlock_error;
            validation_result = false;
            break ;
        }
        field = next_field;
    }
    unlock_error = pt_mutex_unlock_if_not_null(schema_mutable->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (false);
    if (validation_result == true)
    {
        json_schema_set_error_unlocked(schema_mutable, FT_ERR_SUCCESS);
        return (true);
    }
    if (validation_error == FT_ERR_SUCCESS)
        validation_error = schema_mutable->_error_code;
    return (false);
}
