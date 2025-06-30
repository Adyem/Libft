#include <cstring>
#include "json.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

json_group *json_find_group(json_group *head, const char *name)
{
    json_group *current = head;
    while (current)
    {
        if (current->name && ft_strcmp(current->name, name) == 0)
            return (current);
        current = current->next;
    }
    return (ft_nullptr);
}

json_item *json_find_item(json_group *group, const char *key)
{
    if (!group)
        return (ft_nullptr);
    json_item *current = group->items;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
            return (current);
        current = current->next;
    }
    return (ft_nullptr);
}

void json_remove_item(json_group *group, const char *key)
{
    if (!group)
        return ;
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
                delete[] current->key;
            if (current->value)
                delete[] current->value;
            delete current;
            return ;
        }
        previous = current;
        current = current->next;
    }
    return ;
}

void json_update_item(json_group *group, const char *key, const char *value)
{
    if (!group)
        return ;
    json_item *item = json_find_item(group, key);
    if (!item)
        return ;
    if (item->value)
        delete[] item->value;
    item->value = cma_strdup(value);
    if (!item->value)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return ;
    }
    return ;
}

void json_update_item(json_group *group, const char *key, const int value)
{
    if (!group)
        return ;
    json_item *item = json_find_item(group, key);
    if (!item)
        return ;
    if (item->value)
        delete[] item->value;
    item->value = cma_itoa(value);
    if (!item->value)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return ;
    }
    return ;
}

void json_update_item(json_group *group, const char *key, const bool value)
{
    if (!group)
        return ;
    json_item *item = json_find_item(group, key);
    if (!item)
        return ;
    if (item->value)
        delete[] item->value;
    if (value == true)
        item->value = cma_strdup("true");
    else
        item->value = cma_strdup("false");
    if (!item->value)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return ;
    }
    return ;
}

void json_remove_group(json_group **head, const char *name)
{
    if (!head || !(*head))
        return ;
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
                delete[] current->name;
            json_free_items(current->items);
            delete current;
            return ;
        }
        previous = current;
        current = current->next;
    }
    return ;
}
