TARGET := networking.a
DEBUG_TARGET := networking_debug.a

SRCS := networking_socket_class.cpp \
        networking_send_utils.cpp \
        networking.cpp \
        networking_dns_resolver.cpp \
        networking_setup_server.cpp \
        networking_setup_client.cpp \
        networking_setup_udp.cpp \
        networking_udp_event_loop.cpp \
        networking_socket_wrapper_functions.cpp \
        networking_socket_handle.cpp \
        networking_ssl_wrapper.cpp \
        networking_tls_aead.cpp \
        networking_quic_experimental.cpp \
        networking_nonblocking.cpp \
        networking_event_loop.cpp \
        networking_http_client.cpp \
        networking_http2_client.cpp \
        networking_http_server.cpp \
        networking_websocket_client.cpp \
        networking_websocket_server.cpp \
        networking_socket_config_thread_safety.cpp

ifeq ($(OS),Windows_NT)
    SRCS += networking_select.cpp
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        SRCS += networking_epoll.cpp
    else ifeq ($(UNAME_S),Darwin)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),FreeBSD)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),NetBSD)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),OpenBSD)
        SRCS += networking_kqueue.cpp
    else
        SRCS += networking_select.cpp
    endif
endif

HEADERS := socket_class.hpp \
           networking.hpp \
           udp_socket.hpp \
           ssl_wrapper.hpp \
           networking_tls_aead.hpp \
           networking_quic_experimental.hpp \
           http_client.hpp \
           http2_client.hpp \
           http_server.hpp \
           websocket_client.hpp \
           websocket_server.hpp \
           socket_handle.hpp \

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
