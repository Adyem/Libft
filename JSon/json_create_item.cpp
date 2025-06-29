#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <new>
#include "json.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../CMA/CMA.hpp"

json_item* json_create_item(const char *key, const char *value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
	{
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
	}
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    item->value = cma_strdup(value);
    if (!item->value)
    {
        delete[] item->key;
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    item->next = ft_nullptr;
    return (item);
}

json_item* json_create_item(const char *key, const bool value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
	{
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
	}
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
	if (value == true)
		item->value = cma_strdup("true");
	else
		item->value = cma_strdup("false");
    if (!item->value)
    {
        delete[] item->key;
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    item->next = ft_nullptr;
    return (item);
}

json_item* json_create_item(const char *key, const int value)
{
    json_item *item = new(std::nothrow) json_item;
    if (!item)
	{
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
	}
    item->key = cma_strdup(key);
    if (!item->key)
    {
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
	item->value = cma_itoa(value);
    if (!item->value)
    {
        delete[] item->key;
        delete item;
		ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    item->next = ft_nullptr;
    return (item);
}
