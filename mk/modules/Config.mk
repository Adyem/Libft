TARGET := config.a
DEBUG_TARGET := config_debug.a

SRCS :=         config_parse.cpp \
                config_write.cpp \
                config_merge.cpp \
                config_runtime.cpp \
                config_entry_thread_safety.cpp

HEADERS := config.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
