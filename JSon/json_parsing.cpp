#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_big_number.hpp"

static bool json_string_is_integral(const char *value)
{
    if (!value)
        return (false);
    if (!(*value))
        return (false);
    size_t index = 0;
    if (value[index] == '+' || value[index] == '-')
        index++;
    if (!value[index])
        return (false);
    while (value[index])
    {
        if (ft_isdigit(static_cast<unsigned char>(value[index])) == 0)
            return (false);
        index++;
    }
    return (true);
}

static bool json_string_exceeds_signed_long_long(const char *value)
{
    if (json_string_is_integral(value) == false)
        return (false);
    size_t index = 0;
    bool is_negative = false;
    if (value[index] == '+' || value[index] == '-')
    {
        if (value[index] == '-')
            is_negative = true;
        index++;
    }
    while (value[index] == '0' && value[index + 1] != '\0')
        index++;
    const char *digits = value + index;
    size_t digits_length = ft_strlen_size_t(digits);
    if (digits_length == 0)
        return (false);
    if (digits_length > 19)
        return (true);
    if (digits_length < 19)
        return (false);
    if (is_negative)
    {
        if (ft_strcmp(digits, "9223372036854775808") > 0)
            return (true);
        return (false);
    }
    if (ft_strcmp(digits, "9223372036854775807") > 0)
        return (true);
    return (false);
}

void json_item_refresh_numeric_state(json_item *item)
{
    if (!item)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (!item->value)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    if (json_string_exceeds_signed_long_long(item->value) == false)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    ft_big_number *allocated_number = new(std::nothrow) ft_big_number;
    if (!allocated_number)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return ;
    }
    allocated_number->assign(item->value);
    if (allocated_number->get_error() != ER_SUCCESS)
    {
        int error_code = allocated_number->get_error();

        delete allocated_number;
        ft_errno = error_code;
        return ;
    }
    item->big_number = allocated_number;
    item->is_big_number = true;
    ft_errno = ER_SUCCESS;
    return ;
}

void json_add_item_to_group(json_group *group, json_item *item)
{
    if (!group->items)
        group->items = item;
    else
    {
        json_item *current_item = group->items;
        while (current_item->next)
            current_item = current_item->next;
        current_item->next = item;
    }
    return ;
}

json_group* json_create_json_group(const char *name)
{
    json_group *group = new(std::nothrow) json_group;
    if (!group)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    group->name = cma_strdup(name);
    if (!group->name)
    {
        delete group;
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    group->items = ft_nullptr;
    group->next = ft_nullptr;
    ft_errno = ER_SUCCESS;
    return (group);
}

void json_append_group(json_group **head, json_group *new_group)
{
    if (!(*head))
        *head = new_group;
    else
    {
        json_group *current_group = *head;
        while (current_group->next)
            current_group = current_group->next;
        current_group->next = new_group;
    }
    return ;
}

void json_free_items(json_item *item)
{
    while (item)
    {
        json_item *next_item = item->next;
        if (item->key)
            delete[] item->key;
        if (item->value)
            delete[] item->value;
        if (item->big_number)
            delete item->big_number;
        delete item;
        item = next_item;
    }
    return ;
}

void json_free_groups(json_group *group)
{
    while (group)
    {
        json_group *next_group = group->next;
        if (group->name)
            delete[] group->name;
        json_free_items(group->items);
        delete group;
        group = next_group;
    }
    return ;
}
