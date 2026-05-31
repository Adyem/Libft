TARGET := Basic.a
DEBUG_TARGET := Basic_debug.a

SRCS := basic_atoi.cpp \
    basic_nullptr.cpp \
    basic_bzero.cpp \
    basic_memchr.cpp \
    basic_memrchr.cpp \
    basic_memcmp.cpp \
    basic_memcpy.cpp \
    basic_memcpy_s.cpp \
    basic_memmove.cpp \
    basic_memmove_s.cpp \
    basic_memset.cpp \
    basic_strchr.cpp \
    basic_strlcat.cpp \
    basic_strlcpy.cpp \
    basic_strnlen.cpp \
    basic_strncmp.cpp \
    basic_strnstr.cpp \
    basic_strstr.cpp \
    basic_strrchr.cpp \
    basic_strtok.cpp \
    basic_atol.cpp \
    basic_strtol.cpp \
    basic_strtoul.cpp \
    basic_isdigit.cpp \
    basic_isalpha.cpp \
    basic_isalnum.cpp \
    basic_isprint.cpp \
    basic_islower.cpp \
    basic_isupper.cpp \
    basic_strcmp.cpp \
    basic_toupper.cpp \
    basic_tolower.cpp \
    basic_strncpy.cpp \
    basic_strcpy_s.cpp \
    basic_strncpy_s.cpp \
    basic_strcat_s.cpp \
    basic_strncat_s.cpp \
    basic_locale_compare.cpp \
    basic_wstrlen.cpp \
    basic_striteri.cpp \
    basic_isspace.cpp \
    basic_validate_int.cpp \
    basic_utf8_decode.cpp \
    basic_utf8_encode.cpp \
    basic_utf8_length.cpp \
    basic_constant_time_compare.cpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
