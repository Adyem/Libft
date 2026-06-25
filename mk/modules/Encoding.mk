TARGET         := encoding.a
DEBUG_TARGET   := encoding_debug.a

SRCS := encoding.cpp

HEADERS := encoding.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
