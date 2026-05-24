TARGET := Advanced.a
DEBUG_TARGET := Advanced_debug.a

SRCS := advanced_to_string.cpp \
    advanced_wide.cpp \
    advanced_locale_casefold.cpp \
    advanced_span_to_string.cpp \
    advanced_calloc.cpp \
    advanced_atoi.cpp \
    advanced_strdup.cpp \
    advanced_strndup.cpp \
    advanced_memdup.cpp \
    advanced_itoa.cpp \
    advanced_itoa_base.cpp \
    advanced_strjoin.cpp \
    advanced_strjoin_multiple.cpp \
    advanced_substr.cpp \
    advanced_strtrim.cpp \
    advanced_split.cpp

HEADERS := advanced.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
