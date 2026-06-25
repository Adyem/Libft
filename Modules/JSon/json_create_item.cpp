#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

json_item* json_create_item(const char *key, const char *value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = FT_FALSE;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    item->_mutex = ft_nullptr;
    if (json_item_enable_thread_safety(item) != FT_ERR_SUCCESS)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = adv_strdup(key);
    if (!item->key)
    {
        delete item;
        return (ft_nullptr);
    }
    item->value = adv_strdup(value);
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    return (item);
}

json_item* json_create_item(const char *key, const ft_bool value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = FT_FALSE;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    item->_mutex = ft_nullptr;
    if (json_item_enable_thread_safety(item) != FT_ERR_SUCCESS)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = adv_strdup(key);
    if (!item->key)
    {
        delete item;
        return (ft_nullptr);
    }
    if (value == FT_TRUE)
        item->value = adv_strdup("true");
    else
        item->value = adv_strdup("false");
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    return (item);
}

json_item* json_create_item(const char *key, const int32_t value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
    {
        return (ft_nullptr);
    }
    item->key = ft_nullptr;
    item->value = ft_nullptr;
    item->is_big_number = FT_FALSE;
    item->big_number = ft_nullptr;
    item->next = ft_nullptr;
    item->_mutex = ft_nullptr;
    if (json_item_enable_thread_safety(item) != FT_ERR_SUCCESS)
    {
        delete item;
        return (ft_nullptr);
    }
    item->key = adv_strdup(key);
    if (!item->key)
    {
        delete item;
        return (ft_nullptr);
    }
    item->value = adv_itoa(value);
    if (!item->value)
    {
        cma_free(item->key);
        delete item;
        return (ft_nullptr);
    }
    json_item_refresh_numeric_state(item);
    return (item);
}

json_item* json_create_item(const char *key, const ft_big_number &value)
{
    return (json_create_item(key, value.c_str()));
}
