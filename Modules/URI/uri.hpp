#ifndef URI_HPP
# define URI_HPP

#include "../Errno/errno.hpp"
#include <cstdint>

struct uri_components
{
    char    *scheme;
    char    *userinfo;
    char    *host;
    char    *port;
    char    *path;
    char    *query;
    char    *fragment;
};

void        uri_components_reset(uri_components *components) noexcept;
void        uri_components_destroy(uri_components *components) noexcept;
int32_t     uri_parse(const char *input, ft_size_t input_size,
                uri_components *components) noexcept;
char        *uri_normalize(const char *input, ft_size_t input_size);
char        *uri_percent_encode_component(const uint8_t *input,
                ft_size_t input_size, ft_bool keep_slash);
uint8_t     *uri_percent_decode_component(const char *input,
                ft_size_t input_size, ft_size_t *output_size);
char        *uri_query_get_value(const char *query, ft_size_t query_size,
                const char *key, ft_size_t key_size);
int32_t     uri_get_error(void) noexcept;
const char  *uri_get_error_str(void) noexcept;

#endif
