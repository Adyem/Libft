TARGET := GetNextLine.a
DEBUG_TARGET := GetNextLine_debug.a

SRCS := get_next_line.cpp get_next_line_stream.cpp get_next_line_stream_helpers.cpp

HEADERS := get_next_line.hpp gnl_stream.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
