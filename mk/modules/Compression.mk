TARGET := compression.a
DEBUG_TARGET := compression_debug.a

SRCS := Compression_zlib.cpp \
        Compression_stream.cpp \
        Compression_base64.cpp

HEADERS := compression.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
