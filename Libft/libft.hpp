#ifndef LIBFT_HPP
# define LIBFT_HPP

#define FT_SUCCESS 0
#define FT_FAILURE 1

typedef unsigned long long ft_size_t;


#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <type_traits>
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/mutex.hpp"
#include "../CPP_class/class_string_class.hpp"

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
}

constexpr size_t ft_strlen_size_t(const char *string)
{
    if (!ft_is_constant_evaluated())
        ft_errno = ER_SUCCESS;
    if (!string)
    {
        if (!ft_is_constant_evaluated())
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    const char *string_pointer = string;
    while (reinterpret_cast<uintptr_t>(string_pointer) & (sizeof(size_t) - 1))
    {
        if (*string_pointer == '\0')
            return (static_cast<size_t>(string_pointer - string));
        ++string_pointer;
    }
    const size_t *word_pointer = reinterpret_cast<const size_t*>(string_pointer);
    while (!ft_detail::has_zero(*word_pointer))
        ++word_pointer;
    string_pointer = reinterpret_cast<const char*>(word_pointer);
    while (*string_pointer)
        ++string_pointer;
    return (static_cast<size_t>(string_pointer - string));
}

constexpr int ft_strlen(const char *string)
{
    size_t length = 0;

    if (!ft_is_constant_evaluated())
        ft_errno = ER_SUCCESS;
    length = ft_strlen_size_t(string);
    if (length > static_cast<size_t>(FT_INT_MAX))
    {
        if (!ft_is_constant_evaluated())
            ft_errno = FT_ERR_OUT_OF_RANGE;
        return (FT_INT_MAX);
    }
    return (static_cast<int>(length));
}

char            *ft_strchr(const char *string, int char_to_find);
int                ft_atoi(const char *string);
int             ft_validate_int(const char *input);
void            ft_bzero(void *string, size_t size);
void            *ft_memchr(const void *pointer, int character, size_t size);
void            *ft_memcpy(void* destination, const void* source, size_t num);
void            *ft_memmove(void *destination, const void *source, size_t size);
void            *ft_memdup(const void *source, size_t size);
size_t            ft_strlcat(char *destination, const char *source, size_t bufferSize);
size_t            ft_strlcpy(char *destination, const char *source, size_t bufferSize);
size_t            ft_strnlen(const char *string, size_t maximum_length);
char            *ft_strrchr(const char *string, int char_to_find);
char            *ft_strnstr(const char *haystack, const char *needle, size_t length);
char            *ft_strstr(const char *haystack, const char *needle);
int                ft_strncmp(const char *string_1, const char *string_2, size_t max_len);
int                ft_memcmp(const void *pointer1, const void *pointer2, size_t size);
int                ft_isdigit(int character);
int             ft_isalpha(int character);
int             ft_isalnum(int character);
int             ft_isprint(int character);
int             ft_islower(int character);
int             ft_isupper(int character);
long            ft_atol(const char *string);
long            ft_strtol(const char *nptr, char **endptr, int base);
unsigned long   ft_strtoul(const char *nptr, char **endptr, int base);
int                ft_strcmp(const char *string1, const char *string2);
void            ft_to_lower(char *string);
void            ft_to_upper(char *string);
char             *ft_strncpy(char *destination, const char *source, size_t number_of_characters);
char            *ft_strndup(const char *string, size_t maximum_length);
char            *ft_span_dup(const char *buffer, size_t length);
ft_string        ft_span_to_string(const char *buffer, size_t length);
char            *ft_strtok(char *string, const char *delimiters);
void             *ft_memset(void *destination, int value, size_t number_of_bytes);
int             ft_isspace(int character);
char            *ft_strmapi(const char *string, char (*function)(unsigned int, char));
void            ft_striteri(char *string, void (*function)(unsigned int, char *));
char            *ft_getenv(const char *name);
int             ft_setenv(const char *name, const char *value, int overwrite);
int             ft_unsetenv(const char *name);

FILE            *ft_fopen(const char *filename, const char *mode);
int             ft_fclose(FILE *stream);
char            *ft_fgets(char *string, int size, FILE *stream);
int64_t        ft_time_ms(void);
char            *ft_time_format(char *buffer, size_t buffer_size);
ft_string        ft_to_string(long number);

#endif
