TARGET := Sink.a
DEBUG_TARGET := Sink_debug.a

SRCS := \
    sink.cpp

HEADERS := sink.hpp

MODULE_CFLAGS_EXTRA :=

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
