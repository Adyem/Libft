#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

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
        return (ft_nullptr);
    group->name = cma_strdup(name);
    if (!group->name)
    {
        delete group;
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    group->items = ft_nullptr;
    group->next = ft_nullptr;
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
