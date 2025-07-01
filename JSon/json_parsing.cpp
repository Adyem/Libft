#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
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

int json_write_to_file(const char *filename, json_group *groups)
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

char *json_write_to_string(json_group *groups)
{
    char *result = cma_strdup("{\n");
    if (!result)
        return (ft_nullptr);
    json_group *group_ptr = groups;
    while (group_ptr)
    {
        char *line = cma_strjoin_multiple(4, "  \"", group_ptr->name, "\": {\n");
        if (!line)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        char *tmp = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        json_item *item_ptr = group_ptr->items;
        while (item_ptr)
        {
            if (item_ptr->next)
                line = cma_strjoin_multiple(6, "    \"", item_ptr->key, "\": \"", item_ptr->value, "\",\n");
            else
                line = cma_strjoin_multiple(5, "    \"", item_ptr->key, "\": \"", item_ptr->value, "\"\n");
            if (!line)
            {
                cma_free(result);
                return (ft_nullptr);
            }
            tmp = cma_strjoin(result, line);
            cma_free(result);
            cma_free(line);
            if (!tmp)
                return (ft_nullptr);
            result = tmp;
            item_ptr = item_ptr->next;
        }
        if (group_ptr->next)
            line = cma_strdup("  },\n");
        else
            line = cma_strdup("  }\n");
        if (!line)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        group_ptr = group_ptr->next;
    }
    char *end = cma_strdup("}\n");
    if (!end)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    char *tmp = cma_strjoin(result, end);
    cma_free(result);
    cma_free(end);
    if (!tmp)
        return (ft_nullptr);
    result = tmp;
    return (result);
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
