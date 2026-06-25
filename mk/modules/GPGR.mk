TARGET := GPGR.a
DEBUG_TARGET := GPGR_debug.a

SRCS := gpgr_gl_funcs.cpp \
    gpgr_window.cpp \
    gpgr_shader.cpp

MM_SRCS :=

HEADERS := gpgr_gl_funcs.hpp \
    ft_gpu_window.hpp \
    ft_gpu_shader.hpp

ifeq ($(OS),Windows_NT)
    SRCS += gpgr_window_windows.cpp
    HEADERS += gpgr_window_windows.hpp
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    MM_SRCS += gpgr_window_macos.mm
    MODULE_MMFLAGS_EXTRA += -x objective-c++
    HEADERS += gpgr_window_macos.hpp
else
    SRCS += gpgr_window_linux.cpp
    HEADERS += gpgr_window_linux.hpp
endif
endif

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
