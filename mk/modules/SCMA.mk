TARGET         := SCMA.a
DEBUG_TARGET   := SCMA_debug.a

SRCS := scma_allocator.cpp \
scma_debug.cpp \
scma_io.cpp \
scma_mutex.cpp \
scma_state.cpp

HEADERS := SCMA.hpp scma_internal.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
