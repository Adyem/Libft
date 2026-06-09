TARGET := GPGR.a
DEBUG_TARGET := GPGR_debug.a

SRCS := gl_funcs.cpp \
    GpuWindow.cpp \
    GpuShader.cpp

MM_SRCS :=

HEADERS := gl_funcs.hpp \
    GpuWindow.hpp \
    GpuShader.hpp

ifeq ($(OS),Windows_NT)
SRCS += GpuWindow_windows.cpp
HEADERS += GpuWindow_windows.hpp
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
MM_SRCS += GpuWindow_macos.mm
MODULE_MMFLAGS_EXTRA += -x objective-c++
HEADERS += GpuWindow_macos.hpp
else
SRCS += GpuWindow_linux.cpp
HEADERS += GpuWindow_linux.hpp
endif
endif

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
