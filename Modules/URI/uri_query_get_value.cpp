#include "uri_internal.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

static ft_bool uri_query_key_matches(const char *query, ft_size_t key_start,
    ft_size_t key_end, const char *key, ft_size_t key_size)
{
    uint8_t *decoded_key;
    ft_size_t decoded_size;
    ft_bool matches;

    decoded_size = 0;
    decoded_key = uri_percent_decode_component(query + key_start,
            key_end - key_start, &decoded_size);
    if (decoded_key == ft_nullptr)
        return (FT_FALSE);
    matches = FT_FALSE;
    if (decoded_size == key_size
        && ft_memcmp(decoded_key, key, key_size) == 0)
        matches = FT_TRUE;
    cma_free(decoded_key);
    return (matches);
}

char *uri_query_get_value(const char *query, ft_size_t query_size,
    const char *key, ft_size_t key_size)
{
    ft_size_t pair_start;
    ft_size_t pair_end;
    ft_size_t equals_index;
    ft_size_t value_start;
    uint8_t *decoded_value;
    ft_size_t decoded_size;
    char *value;

    if ((query == ft_nullptr && query_size != 0)
        || (key == ft_nullptr && key_size != 0))
    {
        uri_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    pair_start = 0;
    while (pair_start <= query_size)
    {
        pair_end = pair_start;
        while (pair_end < query_size && query[pair_end] != '&')
            pair_end++;
        equals_index = pair_start;
        while (equals_index < pair_end && query[equals_index] != '=')
            equals_index++;
        if (uri_query_key_matches(query, pair_start, equals_index, key,
            key_size) == FT_TRUE)
        {
            value_start = equals_index;
            if (equals_index < pair_end)
                value_start = equals_index + 1;
            decoded_value = uri_percent_decode_component(query + value_start,
                    pair_end - value_start, &decoded_size);
            if (decoded_value == ft_nullptr)
                return (ft_nullptr);
            value = reinterpret_cast<char *>(decoded_value);
            uri_set_error(FT_ERR_SUCCESS);
            return (value);
        }
        if (pair_end == query_size)
            break ;
        pair_start = pair_end + 1;
    }
    uri_set_error(FT_ERR_NOT_FOUND);
    return (ft_nullptr);
}
