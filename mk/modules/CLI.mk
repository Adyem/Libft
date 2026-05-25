TARGET := cli.a
DEBUG_TARGET := cli_debug.a

SRCS := cli.cpp \
        cli_get_bool.cpp \
        cli_get_double.cpp \
        cli_get_int64.cpp \
        cli_get_option.cpp \
        cli_get_string.cpp \
        cli_get_uint64.cpp

HEADERS := cli.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
