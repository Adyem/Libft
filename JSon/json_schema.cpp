#include "json.hpp"
#include "json_schema.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

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
    ft_unique_lock<pt_mutex> schema_guard;
    ft_unique_lock<pt_mutex> field_guard;
    json_schema *schema_mutable;
    json_schema_field *field;
    bool validation_result;

    if (!group)
        return (false);
    schema_mutable = const_cast<json_schema *>(&schema);
    if (json_schema_enable_thread_safety(schema_mutable) != 0)
        return (false);
    if (json_schema_lock(schema_mutable, schema_guard) != FT_ERR_SUCCESSS)
    {
        ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (false);
    }
    field = schema_mutable->fields;
    validation_result = true;
    while (validation_result == true && field)
    {
        json_schema_field *next_field;

        if (json_schema_field_enable_thread_safety(field) != 0)
        {
            json_schema_set_error_unlocked(schema_mutable, ft_errno);
            validation_result = false;
            break;
        }
        if (json_schema_field_lock(field, field_guard) != FT_ERR_SUCCESSS)
        {
            json_schema_set_error_unlocked(schema_mutable, ft_errno);
            validation_result = false;
            break;
        }
        next_field = field->next;
        if (field->key == ft_nullptr)
        {
            json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
            json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
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
                    ft_errno = FT_ERR_NOT_FOUND;
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
                else
                {
                    if (field->type == JSON_BOOL)
                    {
                        if (json_is_bool(item->value) == false)
                            type_invalid = true;
                    }
                }
                if (type_invalid == true)
                {
                    json_schema_field_set_error_unlocked(field, FT_ERR_INVALID_ARGUMENT);
                    json_schema_set_error_unlocked(schema_mutable, FT_ERR_INVALID_ARGUMENT);
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    validation_result = false;
                }
                else
                    json_schema_field_set_error_unlocked(field, FT_ERR_SUCCESSS);
            }
        }
        if (validation_result == false)
            break;
        field = next_field;
    }
    if (validation_result == true)
    {
        json_schema_set_error_unlocked(schema_mutable, FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (true);
    }
    if (ft_errno == FT_ERR_SUCCESSS)
        ft_errno = schema_mutable->_error_code;
    return (false);
}
