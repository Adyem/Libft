#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
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
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int result_code;
    ft_big_number *allocated_number;

    if (!item)
        return ;
    if (json_item_enable_thread_safety(item) != 0)
        return ;
    lock_error = json_item_lock(item, guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    result_code = FT_ERR_SUCCESS;
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (item->value)
    {
        if (json_string_exceeds_signed_long_long(item->value) == true)
        {
            allocated_number = new(std::nothrow) ft_big_number;
            if (!allocated_number)
            {
                result_code = FT_ERR_NO_MEMORY;
            }
            else
            {
                allocated_number->assign(item->value);
                if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
                {
                    result_code = ft_big_number::last_operation_error();
                    delete allocated_number;
                }
                else
                {
                    item->big_number = allocated_number;
                    item->is_big_number = true;
                }
            }
        }
    }
    json_item_set_error_unlocked(item, result_code);
    return ;
}

void json_add_item_to_group(json_group *group, json_item *item)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (group == ft_nullptr || item == ft_nullptr)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_group_lock(group, guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (!group->items)
        group->items = item;
    else
    {
        json_item *current_item = group->items;

        while (current_item->next)
            current_item = current_item->next;
        current_item->next = item;
    }
    json_group_set_error_unlocked(group, FT_ERR_SUCCESS);
    return ;
}

json_group* json_create_json_group(const char *name)
{
    json_group *group = new(std::nothrow) json_group;
    if (!group)
        return (ft_nullptr);
    group->name = adv_strdup(name);
    if (!group->name)
    {
        delete group;
        return (ft_nullptr);
    }
    group->items = ft_nullptr;
    group->next = ft_nullptr;
    if (json_group_enable_thread_safety(group) != 0)
    {
        cma_free(group->name);
        delete group;
        return (ft_nullptr);
    }
    return (group);
}

void json_append_group(json_group **head, json_group *new_group)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (head == ft_nullptr || new_group == ft_nullptr)
    {
        json_group_set_error(new_group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_group_list_lock(guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (!(*head))
        *head = new_group;
    else
    {
        json_group *current_group = *head;

        while (current_group->next)
            current_group = current_group->next;
        current_group->next = new_group;
    }
    json_group_set_error(new_group, FT_ERR_SUCCESS);
    return ;
}

void json_free_items(json_item *item)
{
    while (item)
    {
        json_item *next_item = item->next;
        if (item->key)
            cma_free(item->key);
        if (item->value)
            cma_free(item->value);
        if (item->big_number)
            delete item->big_number;
        delete item;
        item = next_item;
    }
    return ;
}

void json_free_groups(json_group *group)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = json_group_list_lock(guard);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    while (group)
    {
        json_group *next_group = group->next;

        if (group->name)
            cma_free(group->name);
        json_free_items(group->items);
        delete group;
        group = next_group;
    }
    return ;
}
