#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../PThread/pthread_internal.hpp"

static ft_bool json_string_is_integral(const char *value)
{
    if (!value)
        return (FT_FALSE);
    if (!(*value))
        return (FT_FALSE);
    ft_size_t index = 0;
    if (value[index] == '+' || value[index] == '-')
        index++;
    if (!value[index])
        return (FT_FALSE);
    while (value[index])
    {
        if (ft_isdigit(static_cast<unsigned char>(value[index])) == FT_ERR_SUCCESS)
            return (FT_FALSE);
        index++;
    }
    return (FT_TRUE);
}

static ft_bool json_string_exceeds_signed_long_long(const char *value)
{
    if (json_string_is_integral(value) == FT_FALSE)
        return (FT_FALSE);
    ft_size_t index = 0;
    ft_bool is_negative = FT_FALSE;
    if (value[index] == '+' || value[index] == '-')
    {
        if (value[index] == '-')
            is_negative = FT_TRUE;
        index++;
    }
    while (value[index] == '0' && value[index + 1] != '\0')
        index++;
    const char *digits = value + index;
    ft_size_t digits_length = ft_strlen_size_t(digits);
    if (digits_length == FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (digits_length > 19)
        return (FT_TRUE);
    if (digits_length < 19)
        return (FT_FALSE);
    if (is_negative)
    {
        if (ft_strcmp(digits, "9223372036854775808") > 0)
            return (FT_TRUE);
        return (FT_FALSE);
    }
    if (ft_strcmp(digits, "9223372036854775807") > 0)
        return (FT_TRUE);
    return (FT_FALSE);
}

void json_item_refresh_numeric_state(json_item *item)
{
    int32_t lock_error;
    int32_t result_code;
    ft_big_number *allocated_number;

    if (!item)
        return ;
    if (json_item_enable_thread_safety(item) != FT_ERR_SUCCESS)
        return ;
    if (item->_mutex == ft_nullptr)
        return ;
    lock_error = pt_recursive_mutex_lock_if_not_null(item->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    result_code = FT_ERR_SUCCESS;
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = FT_FALSE;
    if (item->value)
    {
        if (json_string_exceeds_signed_long_long(item->value) == FT_TRUE)
        {
            allocated_number = new(std::nothrow) ft_big_number;
            if (!allocated_number)
            {
                result_code = FT_ERR_NO_MEMORY;
            }
            else
            {
                result_code = allocated_number->initialize();
                if (result_code != FT_ERR_SUCCESS)
                {
                    delete allocated_number;
                }
                else
                {
                    allocated_number->assign(item->value);
                    item->big_number = allocated_number;
                    item->is_big_number = FT_TRUE;
                }
            }
        }
    }
    json_item_set_error_unlocked(item, result_code);
    (void)pt_recursive_mutex_unlock_if_not_null(item->_mutex);
    return ;
}

void json_add_item_to_group(json_group *group, json_item *item)
{
    int32_t lock_error;

    if (group == ft_nullptr || item == ft_nullptr)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (group->_mutex == ft_nullptr)
        return ;
    lock_error = pt_recursive_mutex_lock_if_not_null(group->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(group->_mutex);
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
    group->_mutex = ft_nullptr;
    if (json_group_enable_thread_safety(group) != FT_ERR_SUCCESS)
    {
        cma_free(group->name);
        delete group;
        return (ft_nullptr);
    }
    return (group);
}

void json_append_group(json_group **head, json_group *new_group)
{
    int32_t lock_error;

    if (head == ft_nullptr || new_group == ft_nullptr)
    {
        json_group_set_error(new_group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_error = json_group_list_lock_manual();
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
    (void)json_group_list_unlock_manual();
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
        (void)json_item_disable_thread_safety(item);
        delete item;
        item = next_item;
    }
    return ;
}

void json_free_groups(json_group *group)
{
    int32_t lock_error;

    lock_error = json_group_list_lock_manual();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    while (group)
    {
        json_group *next_group = group->next;

        if (group->name)
            cma_free(group->name);
        json_free_items(group->items);
        (void)json_group_disable_thread_safety(group);
        delete group;
        group = next_group;
    }
    (void)json_group_list_unlock_manual();
    return ;
}
