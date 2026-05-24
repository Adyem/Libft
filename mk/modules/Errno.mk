TARGET := errno.a
DEBUG_TARGET := errno_debug.a

SRCS :=        errno_strerror.cpp \
        errno_perror.cpp \
        errno_exit.cpp \
        errno_internal.cpp

HEADERS := errno.hpp errno_internal.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
