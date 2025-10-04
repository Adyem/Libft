#ifndef API_INTERNAL_HPP
#define API_INTERNAL_HPP

#include "../CPP_class/class_string_class.hpp"
#include <cstddef>

bool api_append_content_length_header(ft_string &request, size_t content_length);
size_t api_debug_get_last_async_request_size(void);
size_t api_debug_get_last_async_bytes_sent(void);
int api_debug_get_last_async_send_state(void);
int api_debug_get_last_async_send_timeout(void);
size_t api_debug_get_last_async_bytes_received(void);
int api_debug_get_last_async_receive_state(void);
int api_debug_get_last_async_receive_timeout(void);

#endif
