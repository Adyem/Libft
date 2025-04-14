#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <new>
#include "json_parsing.hpp"
#include "../Printf/printf.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../CMA/CMA.hpp"

json_item* create_json_item(const char *key, const char *value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
        return (ft_nullptr);
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
        return(ft_nullptr);
    }
    item->value = cma_strdup(value);
    if (!item->value)
    {
        delete[] item->key;
        delete item;
        return (ft_nullptr);
    }
    item->next = ft_nullptr;
    return (item);
}

void add_item_to_group(json_group *group, json_item *item)
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

json_group* create_json_group(const char *name)
{
    json_group *group = new(std::nothrow) json_group;
    if (!group)
        return (ft_nullptr);
    group->name = cma_strdup(name);
    if (!group->name)
    {
        delete group;
        return (ft_nullptr);
    }
    group->items = nullptr;
    group->next = nullptr;
    return (group);
}

void append_group(json_group **head, json_group *new_group)
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

int write_json_to_file(const char *filename, json_group *groups)
{
    int file_descriptor = ft_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (-1);
    pf_printf_fd(file_descriptor, "{\n");
    json_group *group_ptr = groups;
    while (group_ptr)
    {
        pf_printf_fd(file_descriptor, "  \"%s\": {\n", group_ptr->name);
        json_item *item_ptr = group_ptr->items;
        while (item_ptr)
        {
            if (item_ptr->next)
                pf_printf_fd(file_descriptor,
						"    \"%s\": \"%s\",\n", item_ptr->key, item_ptr->value);
            else
                pf_printf_fd(file_descriptor,
						"    \"%s\": \"%s\"\n", item_ptr->key, item_ptr->value);
            item_ptr = item_ptr->next;
        }
        if (group_ptr->next)
            pf_printf_fd(file_descriptor, "  },\n");
        else
            pf_printf_fd(file_descriptor, "  }");
        pf_printf_fd(file_descriptor, "\n");
        group_ptr = group_ptr->next;
    }
    pf_printf_fd(file_descriptor, "}\n");
    ft_close(file_descriptor);
    return (0);
}

void free_json_items(json_item *item)
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

void free_json_groups(json_group *group)
{
    while (group)
    {
        json_group *next_group = group->next;
        if (group->name)
            delete[] group->name;
        free_json_items(group->items);
        delete group;
        group = next_group;
    }
    return ;
}
