#include "json.hpp"
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

bool json_validate(json_group *group, const json_schema &schema)
{
    if (!group)
        return (false);
    json_schema_field *field = schema.fields;
    while (field)
    {
        json_item *item = json_find_item(group, field->key);
        if (!item)
        {
            if (field->required == true)
                return (false);
            field = field->next;
            continue;
        }
        if (field->type == JSON_NUMBER)
        {
            if (json_is_number(item->value) == false)
                return (false);
        }
        else
        {
            if (field->type == JSON_BOOL)
            {
                if (json_is_bool(item->value) == false)
                    return (false);
            }
        }
        field = field->next;
    }
    return (true);
}
