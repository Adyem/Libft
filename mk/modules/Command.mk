TARGET := command.a
DEBUG_TARGET := command_debug.a

SRCS :=         command_stack.cpp

HEADERS := command.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
