#ifndef API_HTTP_COMMON_HPP
#define API_HTTP_COMMON_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../JSon/json.hpp"
#include <cstddef>

char api_http_to_lower(char character);
bool api_http_is_whitespace(char character);
bool api_http_line_starts_with(const char *line_start, size_t line_length,
    const char *name);
const char *api_http_find_crlf(const char *start, const char *end);
void api_http_trim_header_value(const char **value_start,
    const char **value_end);
bool api_http_parse_decimal(const char *start, const char *end,
    long long &value);
bool api_http_parse_hex(const char *start, const char *end,
    long long &value);
void api_http_parse_headers(const char *headers_start,
    const char *headers_end, bool &connection_close,
    bool &chunked_encoding, bool &has_length, long long &content_length);
bool api_http_chunked_body_complete(const char *body_start,
    size_t body_size, size_t &consumed_length);
bool api_http_decode_chunked(const char *body_start, size_t body_size,
    ft_string &decoded_body, size_t &consumed_length);

typedef bool (*api_http_send_callback)(const char *data_pointer,
    size_t data_length, void *context, int &error_code);

bool api_http_measure_json_payload(json_group *payload,
    size_t &payload_length);
bool api_http_stream_json_payload(json_group *payload,
    api_http_send_callback send_callback, void *context, int &error_code);

#endif
