TARGET := ReadLine.a
DEBUG_TARGET := ReadLine_debug.a

SRCS := readline.cpp \
    readline_clear_history.cpp \
    readline_handle_keypress.cpp \
    readline_suggestions.cpp \
    readline_utilities.cpp \
    readline_customization.cpp \
    readline_tab_completion.cpp \
    readline_printeble_char.cpp \
    readline_raw_mode.cpp \
    readline_initialize.cpp \
    readline_state.cpp \
    readline_thread_safety.cpp \
    readline_terminal_dimensions.cpp \
    readline_get_terminal_width.cpp
SRCS += readline_utf8.cpp

HEADERS := readline.hpp \
           readline_internal.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
