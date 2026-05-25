ifndef LIBFT_BUILD_CONFIG_INCLUDED
LIBFT_BUILD_CONFIG_INCLUDED := 1

DEMO_OPT_LEVEL ?= 3

CXX             := g++
AR              := ar
ARFLAGS         := rcs
CLANG_FORMAT   ?= clang-format

MAKEFLAGS      += --no-print-directory

SUBMAKE_OVERRIDES ?=

TEMP_DIRS := temp_objs temp_objs_test .libft_output_lock
OUTPUT_LOGS := .libft_build_*.log

ifeq ($(OS),Windows_NT)
    MKDIR  = mkdir
    RM     = del /F /Q
    RMDIR  = rmdir /S /Q
else
    MKDIR  = mkdir -p
    RM     = rm -f
    RMDIR  = rm -rf
endif

SUBDIRS := Modules/Basic \
           Modules/Advanced \
           Modules/Compatebility \
           Modules/Errno \
           Modules/CMA \
           Modules/SCMA \
           Modules/GetNextLine \
           Modules/DUMB \
           Modules/Math \
           Modules/Geometry \
           Modules/System_utils \
           Modules/Printf \
           Modules/ReadLine \
           Modules/PThread \
           Modules/CPP_class \
           Modules/Template \
           Modules/Buffer \
           Modules/CLI \
           Modules/Command \
           Modules/Config \
           Modules/CrossProcess \
           Modules/Compression \
           Modules/Encryption \
           Modules/Encoding \
           Modules/RNG \
           Modules/JSon \
           Modules/YAML \
           Modules/File \
           Modules/HTML \
           Modules/Time \
           Modules/Filesystem \
           Modules/XML \
           Modules/Storage \
           Modules/Networking \
           Modules/URI \
           Modules/API \
           Modules/Observability \
           Modules/Logger \
           Modules/Parser \
           Modules/Game

LIB_BASES := \
  Modules/Basic/Basic \
  Modules/Advanced/Advanced \
  Modules/Compatebility/Compatebility \
  Modules/Errno/errno \
  Modules/CMA/CustomMemoryAllocator \
  Modules/SCMA/SCMA \
  Modules/GetNextLine/GetNextLine \
  Modules/DUMB/dumb \
  Modules/Math/Math \
  Modules/Geometry/geometry \
  Modules/System_utils/System_utils \
  Modules/Printf/Printf \
  Modules/ReadLine/ReadLine \
  Modules/PThread/PThread \
  Modules/CPP_class/CPP_class \
  Modules/Buffer/buffer \
  Modules/CLI/cli \
  Modules/Command/command \
  Modules/Config/config \
  Modules/CrossProcess/CrossProcess \
  Modules/Compression/compression \
  Modules/Encryption/encryption \
  Modules/Encoding/encoding \
  Modules/RNG/RNG \
  Modules/JSon/JSon \
  Modules/YAML/YAML  \
  Modules/File/file \
  Modules/HTML/HTMLParser \
  Modules/Time/time \
  Modules/Filesystem/filesystem \
  Modules/XML/XMLParser \
  Modules/Storage/storage \
  Modules/Networking/networking \
  Modules/URI/uri \
  Modules/API/API \
  Modules/Observability/Observability \
  Modules/Logger/Logger \
  Modules/Parser/parser \
  Modules/Game/Game

LIBS       := $(addsuffix .a, $(LIB_BASES))
DEBUG_LIBS := $(addsuffix _debug.a, $(LIB_BASES))
TEST_LIBS  := $(addsuffix _test.a, $(LIB_BASES))
TOTAL_LIBS := $(words $(LIBS))
TOTAL_DEBUG_LIBS := $(words $(DEBUG_LIBS))
TOTAL_TEST_LIBS := $(words $(TEST_LIBS))

TARGET        := Full_Libft.a
DEBUG_TARGET  := Full_Libft_debug.a
TEST_TARGET   := Full_Libft_test.a
TEST_DEBUG_TARGET := Full_Libft_test_debug.a

CPP_CLASS_LIB := Modules/CPP_class/CPP_class.a

endif
