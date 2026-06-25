TARGET := dumb.a
DEBUG_TARGET := dumb_debug.a

SRCS := dumb_io.cpp \
    dumb_controls.cpp \
    dumb_sound.cpp \
    dumb_render.cpp \
    dumb_sound_clip.cpp \
    dumb_console.cpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
