TARGET := compression.a
DEBUG_TARGET := compression_debug.a

UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Linux)
SRCS := Compression_base64.cpp

HAS_ZLIB_HEADERS := $(shell printf "\#include <zlib.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(HAS_ZLIB_HEADERS),1)
SRCS += Compression_zlib.cpp \
        Compression_stream.cpp
endif
else
SRCS := Compression_zlib.cpp \
        Compression_stream.cpp \
        Compression_base64.cpp
endif

HEADERS := compression.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
