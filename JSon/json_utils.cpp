#include <cstring>
#include "json.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

json_group *json_find_group(json_group *head, const char *name)
{
    if (!name)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    json_group *current = head;
    while (current)
    {
        if (current->name && ft_strcmp(current->name, name) == 0)
        {
            ft_errno = ER_SUCCESS;
            return (current);
        }
        current = current->next;
    }
    ft_errno = ER_SUCCESS;
    return (ft_nullptr);
}

json_item *json_find_item(json_group *group, const char *key)
{
    if (!group || !key)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    json_item *current = group->items;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
        {
            ft_errno = ER_SUCCESS;
            return (current);
        }
        current = current->next;
    }
    ft_errno = ER_SUCCESS;
    return (ft_nullptr);
}

void json_remove_item(json_group *group, const char *key)
{
    if (!group || !key)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_item *current = group->items;
    json_item *previous = ft_nullptr;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
        {
            if (previous)
                previous->next = current->next;
            else
                group->items = current->next;
            if (current->key)
                cma_free(current->key);
            if (current->value)
                cma_free(current->value);
            if (current->big_number)
                delete current->big_number;
            delete current;
            ft_errno = ER_SUCCESS;
            return ;
        }
        previous = current;
        current = current->next;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

void json_update_item(json_group *group, const char *key, const char *value)
{
    if (!group || !key || !value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (item->value)
        cma_free(item->value);
    item->value = cma_strdup(value);
    if (!item->value)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    ft_errno = ER_SUCCESS;
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const int value)
{
    if (!group || !key)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (item->value)
        cma_free(item->value);
    item->value = cma_itoa(value);
    if (!item->value)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    ft_errno = ER_SUCCESS;
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const bool value)
{
    if (!group || !key)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (item->value)
        cma_free(item->value);
    if (value == true)
        item->value = cma_strdup("true");
    else
        item->value = cma_strdup("false");
    if (!item->value)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    ft_errno = ER_SUCCESS;
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const ft_big_number &value)
{
    if (!group || !key)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_item *item = json_find_item(group, key);
    if (!item)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (item->big_number)
    {
        delete item->big_number;
        item->big_number = ft_nullptr;
    }
    item->is_big_number = false;
    if (item->value)
        cma_free(item->value);
    item->value = cma_strdup(value.c_str());
    if (!item->value)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    ft_errno = ER_SUCCESS;
    json_item_refresh_numeric_state(item);
    return ;
}

void json_remove_group(json_group **head, const char *name)
{
    if (!head || !(*head) || !name)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    json_group *current = *head;
    json_group *previous = ft_nullptr;
    while (current)
    {
        if (current->name && ft_strcmp(current->name, name) == 0)
        {
            if (previous)
                previous->next = current->next;
            else
                *head = current->next;
            if (current->name)
                cma_free(current->name);
            json_free_items(current->items);
            delete current;
            ft_errno = ER_SUCCESS;
            return ;
        }
        previous = current;
        current = current->next;
    }
    ft_errno = ER_SUCCESS;
    return ;
}
