#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl = false);
int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl = false);

#endif
