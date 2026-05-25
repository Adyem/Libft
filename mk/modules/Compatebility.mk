TARGET := Compatebility.a
DEBUG_TARGET := Compatebility_debug.a

SRCS := Compatebility_file_io.cpp Compatebility_file_ops.cpp Compatebility_file_watch.cpp Compatebility_file_dir.cpp Compatebility_file_path.cpp Compatebility_path_canonical.cpp Compatebility_rng.cpp Compatebility_readline.cpp Compatebility_pthread.cpp Compatebility_system.cpp Compatebility_write.cpp Compatebility_syslog.cpp Compatebility_networking.cpp Compatebility_time.cpp Compatebility_cross_process_posix.cpp Compatebility_cross_process_windows.cpp Compatebility_cma_platform.cpp Compatebility_service.cpp Compatebility_storage_memory_mapped.cpp Compatebility_stack_trace.cpp
MM_SRCS :=
HEADERS := compatebility_internal.hpp \
        compatebility_cma_platform.hpp \
        compatebility_cross_process.hpp \
        compatebility_stack_trace.hpp

ifeq ($(OS),Windows_NT)
SRCS += Compatebility_dumb_render_win32.cpp
SRCS += Compatebility_dumb_controls_win32.cpp
SRCS += Compatebility_dumb_sound_win32.cpp
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
MM_SRCS += Compatebility_dumb_render_platform_macos.mm
MM_SRCS += Compatebility_dumb_controls_platform_macos.mm
MODULE_MMFLAGS_EXTRA += -x objective-c++
SRCS += Compatebility_dumb_sound_macos_coreaudio.cpp
else
SRCS += Compatebility_dumb_render_linux_x11.cpp
SRCS += Compatebility_dumb_controls_linux_x11.cpp
SRCS += Compatebility_dumb_sound_linux_alsa.cpp
endif
endif

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
