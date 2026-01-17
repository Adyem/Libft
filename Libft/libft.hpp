#ifndef LIBFT_HPP
# define LIBFT_HPP

#define FT_SUCCESS 0
#define FT_FAILURE 1

#ifndef FT_TYPES_HPP
# define FT_TYPES_HPP

typedef unsigned long long ft_size_t;

#endif

#include "libft_config.hpp"
#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <wchar.h>
#include "limits.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_string.hpp"

class ft_string;

constexpr bool ft_is_constant_evaluated()
{
#if defined(__cpp_lib_is_constant_evaluated)
    return (std::is_constant_evaluated());
#elif defined(__has_builtin)
# if __has_builtin(__builtin_is_constant_evaluated)
    return (__builtin_is_constant_evaluated());
# else
    return (false);
# endif
#elif defined(__GNUC__) || defined(__clang__)
    return (__builtin_is_constant_evaluated());
#else
    return (false);
#endif
}

namespace ft_detail
{
    constexpr size_t repeat_byte(size_t value)
    {
        return (~static_cast<size_t>(0) / 0xFF * value);
    }

    constexpr bool has_zero(size_t value)
    {
        return (((value) - repeat_byte(0x01)) & ~(value) & repeat_byte(0x80)) != 0;
    }

    constexpr size_t strlen_raw(const char *string)
    {
        const char *string_pointer = string;

        while (*string_pointer)
            ++string_pointer;
        return (static_cast<size_t>(string_pointer - string));
    }
}

constexpr size_t ft_strlen_size_t(const char *string)
{
    if (!string)
    {
        if (!ft_is_constant_evaluated())
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        }
        return (0);
    }
    if (!ft_is_constant_evaluated())
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (ft_detail::strlen_raw(string));
}

constexpr int ft_strlen(const char *string)
{
    size_t length = 0;

    if (!string)
    {
        if (!ft_is_constant_evaluated())
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        }
        return (0);
    }
    length = ft_detail::strlen_raw(string);
    if (length > static_cast<size_t>(FT_INT_MAX))
    {
        if (!ft_is_constant_evaluated())
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        }
        return (FT_INT_MAX);
    }
    if (!ft_is_constant_evaluated())
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (static_cast<int>(length));
}

char            *ft_strchr(const char *string, int char_to_find);
int                ft_atoi(const char *string);
int             ft_validate_int(const char *input);
void            ft_bzero(void *string, size_t size);
void            *ft_memchr(const void *pointer, int character, size_t size);
void            *ft_memcpy(void* destination, const void* source, size_t size);
#if LIBFT_HAS_BOUNDS_CHECKED_HELPERS
int             ft_memcpy_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes);
#endif
void            *ft_memmove(void *destination, const void *source, size_t size);
#if LIBFT_HAS_BOUNDS_CHECKED_HELPERS
int             ft_memmove_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes);
#endif
size_t            ft_strlcat(char *destination, const char *source, size_t buffer_size);
size_t            ft_strlcpy(char *destination, const char *source, size_t buffer_size);
size_t            ft_strnlen(const char *string, size_t maximum_length);
char            *ft_strrchr(const char *string, int char_to_find);
char            *ft_strnstr(const char *haystack, const char *needle, size_t maximum_length);
char            *ft_strstr(const char *haystack, const char *needle);
int                ft_strncmp(const char *string_1, const char *string_2, size_t maximum_length);
int                ft_memcmp(const void *pointer1, const void *pointer2, size_t size);
int                ft_isdigit(int character);
int             ft_isalpha(int character);
int             ft_isalnum(int character);
int             ft_isprint(int character);
int             ft_islower(int character);
int             ft_isupper(int character);
long            ft_atol(const char *string);
long            ft_strtol(const char *input_string, char **end_pointer, int numeric_base);
unsigned long   ft_strtoul(const char *input_string, char **end_pointer, int numeric_base);
int                ft_strcmp(const char *string1, const char *string2);
void            ft_to_lower(char *string);
void            ft_to_upper(char *string);
char             *ft_strncpy(char *destination, const char *source, size_t number_of_characters);
#if LIBFT_HAS_BOUNDS_CHECKED_HELPERS
int             ft_strcpy_s(char *destination, size_t destination_size, const char *source);
int             ft_strncpy_s(char *destination, size_t destination_size, const char *source, size_t maximum_copy_length);
int             ft_strcat_s(char *destination, size_t destination_size, const char *source);
int             ft_strncat_s(char *destination, size_t destination_size, const char *source, size_t maximum_append_length);
#endif
char            *ft_span_dup(const char *buffer, size_t length);
ft_string        ft_span_to_string(const char *buffer, size_t length);
char            *ft_strtok(char *string, const char *delimiters);
#if LIBFT_HAS_LOCALE_HELPERS
int             ft_locale_compare(const char *left, const char *right, const char *locale_name);
ft_string        ft_locale_casefold(const char *input, const char *locale_name);
#endif
void             *ft_memset(void *destination, int value, size_t number_of_bytes);
int             ft_isspace(int character);
char            *ft_strmapi(const char *string, char (*function)(unsigned int, char));
void            ft_striteri(char *string, void (*function)(unsigned int, char *));
#if LIBFT_HAS_ENVIRONMENT_HELPERS
char            *ft_getenv(const char *name);
int             ft_setenv(const char *name, const char *value, int overwrite);
int             ft_unsetenv(const char *name);
#endif
size_t          ft_wstrlen(const wchar_t *string);
ft_string       ft_utf16_to_utf8(const char16_t *input, size_t input_length);
ft_string       ft_utf32_to_utf8(const char32_t *input, size_t input_length);
char16_t        *ft_utf8_to_utf16(const char *input, size_t input_length, size_t *output_length_pointer);
char32_t        *ft_utf8_to_utf32(const char *input, size_t input_length, size_t *output_length_pointer);

#if LIBFT_HAS_FILE_IO_HELPERS
FILE            *ft_fopen(const char *filename, const char *mode);
int             ft_fclose(FILE *stream);
char            *ft_fgets(char *string, int size, FILE *stream);
#endif
#if LIBFT_HAS_TIME_HELPERS
int64_t        ft_time_ms(void);
char            *ft_time_format(char *buffer, size_t buffer_size);
#endif
ft_string        ft_to_string(long number);
ft_string        ft_to_string(unsigned long number);
ft_string        ft_to_string(unsigned long long number);
ft_string        ft_to_string(double number);
ft_string        ft_to_string(int number);
ft_string        ft_to_string(unsigned int number);
ft_string        ft_to_string(float number);

#endif
