TARGET := GPGR.a
DEBUG_TARGET := GPGR_debug.a

GPGR_COMMON_SRCS := gpgr_gl_funcs.cpp \
    gpgr_window.cpp \
    gpgr_shader.cpp

SRCS := $(GPGR_COMMON_SRCS)
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
    HAS_GPGR_LINUX_HEADERS := $(shell printf "\#include <GL/gl.h>\n\#include <GL/glext.h>\n\#include <GL/glx.h>\n\#include <X11/Xatom.h>\n\#include <X11/Xlib.h>\n\#include <X11/keysym.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
    ifeq ($(HAS_GPGR_LINUX_HEADERS),1)
        SRCS += gpgr_window_linux.cpp
        HEADERS += gpgr_window_linux.hpp
    else
        SRCS :=
        HEADERS :=
    endif
endif
endif

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
