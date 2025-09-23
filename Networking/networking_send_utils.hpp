#ifndef NETWORKING_SEND_UTILS_HPP
#define NETWORKING_SEND_UTILS_HPP

#include "ssl_wrapper.hpp"

int networking_check_socket_after_send(int socket_fd);
int networking_check_ssl_after_send(SSL *ssl_connection);

#endif
