TARGET := CrossProcess.a
DEBUG_TARGET := CrossProcess_debug.a

SRCS := CrossProcess_send_descriptor.cpp \
        CrossProcess_receive_descriptor.cpp \
        CrossProcess_receive_memory.cpp \
        CrossProcess_write_memory.cpp

HEADERS := cross_process.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
