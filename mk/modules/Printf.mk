TARGET := Printf.a
DEBUG_TARGET := Printf_debug.a

SRCS := printf_printf.cpp \
                printf_format.cpp \
                printf_print_args.cpp \
                printf_ft_fprintf.cpp \
                printf_custom_specifier.cpp \
                printf_snprintf.cpp \
                printf_vsnprintf.cpp \
                printf_engine.cpp

HEADERS := printf.hpp \
           printf_internal.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
