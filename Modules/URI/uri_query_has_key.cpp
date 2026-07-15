#include "uri.hpp"
#include "../CMA/CMA.hpp"

ft_bool uri_query_has_key(const char *query, ft_size_t query_size,
    const char *key, ft_size_t key_size)
{
    char *value;

    if ((query == ft_nullptr && query_size != 0)
        || (key == ft_nullptr && key_size != 0))
        return (FT_FALSE);
    value = uri_query_get_value(query, query_size, key, key_size);
    if (value == ft_nullptr)
        return (FT_FALSE);
    cma_free(value);
    return (FT_TRUE);
}
