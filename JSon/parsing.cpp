#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <new>  // for std::nothrow
#include "parsing.hpp"
#include "../Libft/libft.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../CMA/CMA.hpp"

json_item* create_json_item(const char *key, const char *value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        std::fprintf(stderr, "Error allocating memory for json_item\n");
        std::exit(EXIT_FAILURE);
    }
    item->key = cma_strdup(key);
    if (!item->key)
    {
        std::fprintf(stderr, "Error allocating memory for json_item key\n");
        delete item;
        std::exit(EXIT_FAILURE);
    }
    item->value = cma_strdup(value);
    if (!item->value)
    {
        std::fprintf(stderr, "Error allocating memory for json_item value\n");
        delete[] item->key;
        delete item;
        std::exit(EXIT_FAILURE);
    }
    item->next = nullptr;
    return item;
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
    return;
}

json_group* create_json_group(const char *name)
{
    json_group *group = new(std::nothrow) json_group;
    if (!group)
    {
        std::fprintf(stderr, "Error allocating memory for json_group\n");
        return ft_nullptr;
    }
    group->name = cma_strdup(name);
    if (!group->name)
    {
        std::fprintf(stderr, "Error allocating memory for json_group name\n");
        delete group;
        return ft_nullptr;
    }
    group->items = nullptr;
    group->next = nullptr;
    return group;
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
    return;
}

int write_json_to_file(const char *filename, json_group *groups)
{
    int file_descriptor = ft_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
    {
        std::fprintf(stderr, "Error: could not open file %s for writing.\n", filename);
        return (-1);
    }
    char buffer[1024];
    std::snprintf(buffer, sizeof(buffer), "{\n");
    ft_write(file_descriptor, buffer, ft_strlen(buffer));
    json_group *group_ptr = groups;
    while (group_ptr)
    {
        std::snprintf(buffer, sizeof(buffer), "  \"%s\": {\n", group_ptr->name);
        ft_write(file_descriptor, buffer, ft_strlen(buffer));
        
        json_item *item_ptr = group_ptr->items;
        while (item_ptr)
        {
            if (item_ptr->next)
                std::snprintf(buffer, sizeof(buffer),
                              "    \"%s\": \"%s\",\n", item_ptr->key, item_ptr->value);
            else
                std::snprintf(buffer, sizeof(buffer),
                              "    \"%s\": \"%s\"\n", item_ptr->key, item_ptr->value);
            ft_write(file_descriptor, buffer, ft_strlen(buffer));
            item_ptr = item_ptr->next;
        }
        
        if (group_ptr->next)
            std::snprintf(buffer, sizeof(buffer), "  },\n");
        else
            std::snprintf(buffer, sizeof(buffer), "  }");
        ft_write(file_descriptor, buffer, ft_strlen(buffer));
        
        std::snprintf(buffer, sizeof(buffer), "\n");
        ft_write(file_descriptor, buffer, ft_strlen(buffer));
        group_ptr = group_ptr->next;
    }
    std::snprintf(buffer, sizeof(buffer), "}\n");
    ft_write(file_descriptor, buffer, ft_strlen(buffer));
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
    return;
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
    return;
}
