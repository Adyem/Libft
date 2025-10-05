#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"
#include "ssl_wrapper.hpp"

/*
** http_response_handler callbacks receive the numeric status code, the
** complete HTTP header block (including terminating CRLF sequences) and body
** data as it is read from the network. The finished flag is set to true once
** the stream ends. Callbacks must avoid blocking to keep the receive loop
** responsive.
*/
typedef void (*http_response_handler)(int status_code, const ft_string &headers,
    const char *body_chunk, size_t chunk_size, bool finished);

int http_get_stream(const char *host, const char *path, http_response_handler handler,
    bool use_ssl = false, const char *custom_port = NULL);
int http_get(const char *host, const char *path, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length);
int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length);

#endif
