TARGET         := buffer.a
DEBUG_TARGET   := buffer_debug.a

SRCS := buffer_byte_buffer.cpp

HEADERS := buffer.hpp \
           byte_buffer.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
