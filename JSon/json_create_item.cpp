#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CMA/CMA.hpp"

json_item* json_create_item(const char *key, const char *value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = false;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    if (json_item_enable_thread_safety(item) != 0)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    item->value = cma_strdup(value);
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    ft_errno = FT_ERR_SUCCESSS;
    return (item);
}

json_item* json_create_item(const char *key, const bool value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = false;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    if (json_item_enable_thread_safety(item) != 0)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (value == true)
        item->value = cma_strdup("true");
    else
        item->value = cma_strdup("false");
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    ft_errno = FT_ERR_SUCCESSS;
    return (item);
}

json_item* json_create_item(const char *key, const int value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = false;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    if (json_item_enable_thread_safety(item) != 0)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    item->value = cma_itoa(value);
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    ft_errno = FT_ERR_SUCCESSS;
    return (item);
}

json_item* json_create_item(const char *key, const ft_big_number &value)
{
    return (json_create_item(key, value.c_str()));
}
