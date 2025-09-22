#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"
#include "ssl_wrapper.hpp"

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length);
int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length);

#endif
