TARGET := uri.a
DEBUG_TARGET := uri_debug.a

SRCS := uri_internal.cpp \
        uri_components_reset.cpp \
        uri_components_destroy.cpp \
        uri_parse.cpp \
        uri_normalize.cpp \
        uri_percent_encode_component.cpp \
        uri_percent_decode_component.cpp \
        uri_query_get_value.cpp

HEADERS := uri.hpp uri_internal.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
