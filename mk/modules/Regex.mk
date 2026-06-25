TARGET := regex.a
DEBUG_TARGET := regex_debug.a

SRCS := regex.cpp

HEADERS := regex.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
