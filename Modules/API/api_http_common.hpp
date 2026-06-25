#ifndef API_HTTP_COMMON_HPP
#define API_HTTP_COMMON_HPP

#include "../Basic/limits.hpp"
#include "../CPP_class/class_string.hpp"
#include "../JSon/json.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstddef>

char api_http_to_lower(char character);
ft_bool api_http_is_whitespace(char character);
ft_bool api_http_line_starts_with(const char *line_start, ft_size_t line_length,
    const char *name);
const char *api_http_find_crlf(const char *start, const char *end);
void api_http_trim_header_value(const char **value_start,
    const char **value_end);
ft_bool api_http_parse_decimal(const char *start, const char *end,
    int64_t &value);
ft_bool api_http_parse_hex(const char *start, const char *end,
    int64_t &value);
void api_http_parse_headers(const char *headers_start,
    const char *headers_end, ft_bool &connection_close,
    ft_bool &chunked_encoding, ft_bool &has_length, int64_t &content_length);
ft_bool api_http_chunked_body_complete(const char *body_start,
    ft_size_t body_size, ft_size_t &consumed_length);
ft_bool api_http_decode_chunked(const char *body_start, ft_size_t body_size,
    ft_string &decoded_body, ft_size_t &consumed_length);

typedef ft_bool (*api_http_send_callback)(const char *data_pointer,
    ft_size_t data_length, void *context, int32_t &error_code);

ft_bool api_http_measure_json_payload(json_group *payload,
    ft_size_t &payload_length);
ft_bool api_http_stream_json_payload(json_group *payload,
    api_http_send_callback send_callback, void *context, int32_t &error_code);

#endif
