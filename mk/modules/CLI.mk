TARGET := cli.a
DEBUG_TARGET := cli_debug.a

SRCS :=         cli.cpp

HEADERS := cli.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
