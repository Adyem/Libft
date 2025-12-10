#include <cstring>
#include "json.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static json_item *json_find_item_locked(json_group *group, const char *key)
{
    json_item *current;

    current = group->items;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
            return (current);
        current = current->next;
    }
    return (ft_nullptr);
}

json_group *json_find_group(json_group *head, const char *name)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    json_group *current;

    if (name == ft_nullptr)
    {
        json_group_set_error(head, FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    entry_errno = ft_errno;
    if (json_group_list_lock(guard) != FT_ER_SUCCESSS)
        return (ft_nullptr);
    current = head;
    while (current)
    {
        if (current->name && ft_strcmp(current->name, name) == 0)
        {
            json_group_list_restore(guard, entry_errno);
            json_group_set_error(current, FT_ER_SUCCESSS);
            return (current);
        }
        current = current->next;
    }
    json_group_list_restore(guard, entry_errno);
    json_group_set_error(head, FT_ERR_NOT_FOUND);
    ft_errno = FT_ERR_NOT_FOUND;
    return (ft_nullptr);
}

json_item *json_find_item(json_group *group, const char *key)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    json_item *current;

    if (group == ft_nullptr || key == ft_nullptr)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return (ft_nullptr);
    current = group->items;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
        {
            json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
            json_group_restore_errno(group, guard, entry_errno);
            return (current);
        }
        current = current->next;
    }
    json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
    json_group_restore_errno(group, guard, entry_errno);
    ft_errno = FT_ERR_NOT_FOUND;
    return (ft_nullptr);
}

void json_remove_item(json_group *group, const char *key)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    json_item *current;
    json_item *previous;

    if (group == ft_nullptr || key == ft_nullptr)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return ;
    current = group->items;
    previous = ft_nullptr;
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
            json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
            json_group_restore_errno(group, guard, entry_errno);
            return ;
        }
        previous = current;
        current = current->next;
    }
    json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
    json_group_restore_errno(group, guard, entry_errno);
    ft_errno = FT_ERR_NOT_FOUND;
    return ;
}

void json_update_item(json_group *group, const char *key, const char *value)
{
    ft_unique_lock<pt_mutex> guard;
    ft_unique_lock<pt_mutex> item_guard;
    int entry_errno;
    int item_entry_errno;
    json_item *item;

    if (!group || !key || !value)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return ;
    item = json_find_item_locked(group, key);
    if (!item)
    {
        json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NOT_FOUND;
        return ;
    }
    if (json_item_enable_thread_safety(item) != 0)
    {
        json_group_set_error_unlocked(group, ft_errno);
        json_group_restore_errno(group, guard, entry_errno);
        return ;
    }
    item_entry_errno = ft_errno;
    if (json_item_lock(item, item_guard) != FT_ER_SUCCESSS)
    {
        json_group_restore_errno(group, guard, entry_errno);
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
        json_item_set_error_unlocked(item, FT_ERR_NO_MEMORY);
        json_item_restore_errno(item, item_guard, item_entry_errno);
        json_group_set_error_unlocked(group, FT_ERR_NO_MEMORY);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    json_item_set_error_unlocked(item, FT_ER_SUCCESSS);
    json_item_restore_errno(item, item_guard, item_entry_errno);
    json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
    json_group_restore_errno(group, guard, entry_errno);
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const int value)
{
    ft_unique_lock<pt_mutex> guard;
    ft_unique_lock<pt_mutex> item_guard;
    int entry_errno;
    int item_entry_errno;
    json_item *item;

    if (!group || !key)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return ;
    item = json_find_item_locked(group, key);
    if (!item)
    {
        json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NOT_FOUND;
        return ;
    }
    if (json_item_enable_thread_safety(item) != 0)
    {
        json_group_set_error_unlocked(group, ft_errno);
        json_group_restore_errno(group, guard, entry_errno);
        return ;
    }
    item_entry_errno = ft_errno;
    if (json_item_lock(item, item_guard) != FT_ER_SUCCESSS)
    {
        json_group_restore_errno(group, guard, entry_errno);
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
        json_item_set_error_unlocked(item, FT_ERR_NO_MEMORY);
        json_item_restore_errno(item, item_guard, item_entry_errno);
        json_group_set_error_unlocked(group, FT_ERR_NO_MEMORY);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    json_item_set_error_unlocked(item, FT_ER_SUCCESSS);
    json_item_restore_errno(item, item_guard, item_entry_errno);
    json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
    json_group_restore_errno(group, guard, entry_errno);
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const bool value)
{
    ft_unique_lock<pt_mutex> guard;
    ft_unique_lock<pt_mutex> item_guard;
    int entry_errno;
    int item_entry_errno;
    json_item *item;

    if (!group || !key)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return ;
    item = json_find_item_locked(group, key);
    if (!item)
    {
        json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NOT_FOUND;
        return ;
    }
    if (json_item_enable_thread_safety(item) != 0)
    {
        json_group_set_error_unlocked(group, ft_errno);
        json_group_restore_errno(group, guard, entry_errno);
        return ;
    }
    item_entry_errno = ft_errno;
    if (json_item_lock(item, item_guard) != FT_ER_SUCCESSS)
    {
        json_group_restore_errno(group, guard, entry_errno);
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
        json_item_set_error_unlocked(item, FT_ERR_NO_MEMORY);
        json_item_restore_errno(item, item_guard, item_entry_errno);
        json_group_set_error_unlocked(group, FT_ERR_NO_MEMORY);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    json_item_set_error_unlocked(item, FT_ER_SUCCESSS);
    json_item_restore_errno(item, item_guard, item_entry_errno);
    json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
    json_group_restore_errno(group, guard, entry_errno);
    json_item_refresh_numeric_state(item);
    return ;
}

void json_update_item(json_group *group, const char *key, const ft_big_number &value)
{
    ft_unique_lock<pt_mutex> guard;
    ft_unique_lock<pt_mutex> item_guard;
    int entry_errno;
    int item_entry_errno;
    json_item *item;

    if (!group || !key)
    {
        json_group_set_error(group, FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_lock(group, guard) != FT_ER_SUCCESSS)
        return ;
    item = json_find_item_locked(group, key);
    if (!item)
    {
        json_group_set_error_unlocked(group, FT_ERR_NOT_FOUND);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NOT_FOUND;
        return ;
    }
    if (json_item_enable_thread_safety(item) != 0)
    {
        json_group_set_error_unlocked(group, ft_errno);
        json_group_restore_errno(group, guard, entry_errno);
        return ;
    }
    item_entry_errno = ft_errno;
    if (json_item_lock(item, item_guard) != FT_ER_SUCCESSS)
    {
        json_group_restore_errno(group, guard, entry_errno);
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
        json_item_set_error_unlocked(item, FT_ERR_NO_MEMORY);
        json_item_restore_errno(item, item_guard, item_entry_errno);
        json_group_set_error_unlocked(group, FT_ERR_NO_MEMORY);
        json_group_restore_errno(group, guard, entry_errno);
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    json_item_set_error_unlocked(item, FT_ER_SUCCESSS);
    json_item_restore_errno(item, item_guard, item_entry_errno);
    json_group_set_error_unlocked(group, FT_ER_SUCCESSS);
    json_group_restore_errno(group, guard, entry_errno);
    json_item_refresh_numeric_state(item);
    return ;
}

void json_remove_group(json_group **head, const char *name)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    json_group *current;
    json_group *previous;

    if (!head || !(*head) || !name)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    entry_errno = ft_errno;
    if (json_group_list_lock(guard) != FT_ER_SUCCESSS)
        return ;
    current = *head;
    previous = ft_nullptr;
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
            json_group_list_restore(guard, entry_errno);
            ft_errno = FT_ER_SUCCESSS;
            return ;
        }
        previous = current;
        current = current->next;
    }
    json_group_list_restore(guard, entry_errno);
    ft_errno = FT_ERR_NOT_FOUND;
    return ;
}
