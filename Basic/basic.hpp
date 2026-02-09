#ifndef LIBFT_HPP
# define LIBFT_HPP

#include <cstdint>
#define FT_SUCCESS 0
#define FT_FAILURE 1

#ifndef FT_TYPES_HPP
# define FT_TYPES_HPP

typedef unsigned long long ft_size_t;

#endif

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <wchar.h>
 #include "limits.hpp"

 class ft_string;
static constexpr size_t ft_strlen_raw(const char *string)
{
    const char *string_pointer = string;

    while (*string_pointer)
        ++string_pointer;
    return (static_cast<size_t>(string_pointer - string));
}

constexpr size_t ft_strlen_size_t(const char *string)
{
    if (!string)
        return (0);
    return (ft_strlen_raw(string));
}

constexpr int ft_strlen(const char *string)
{
    if (!string)
        return (0);
    size_t length = ft_strlen_raw(string);
    if (length > static_cast<size_t>(FT_INT_MAX))
        return (FT_INT_MAX);
    return (static_cast<int>(length));
}

char            *ft_strchr(const char *string, int char_to_find);
int32_t         ft_atoi(const char *string);
int             ft_validate_int(const char *input);
void            ft_bzero(void *string, size_t size);
void            *ft_memchr(const void *pointer, int character, size_t size);
void            *ft_memcpy(void *destination, const void *source, size_t size);
int             ft_memcpy_s(void *destination, size_t destination_size,
                    const void *source, size_t number_of_bytes);
void            *ft_memmove(void *destination, const void *source, size_t size);
int             ft_memmove_s(void *destination, size_t destination_size,
                    const void *source, size_t number_of_bytes);
size_t          ft_strlcat(char *destination, const char *source, size_t buffer_size);
size_t          ft_strlcpy(char *destination, const char *source, size_t buffer_size);
size_t          ft_strnlen(const char *string, size_t maximum_length);
char            *ft_strrchr(const char *string, int char_to_find);
char            *ft_strnstr(const char *haystack, const char *needle,
                    size_t maximum_length);
char            *ft_strstr(const char *haystack, const char *needle);
int             ft_strncmp(const char *string_1, const char *string_2,
                    size_t maximum_length);
int             ft_memcmp(const void *pointer1, const void *pointer2, size_t size);
int             ft_isdigit(int character);
int             ft_isalpha(int character);
int             ft_isalnum(int character);
int             ft_isprint(int character);
int             ft_islower(int character);
int             ft_isupper(int character);
int64_t         ft_atol(const char *string);
long            ft_strtol(const char *input_string, char **end_pointer, int numeric_base);
unsigned long   ft_strtoul(const char *input_string,
                    char **end_pointer, int numeric_base);
int             ft_strcmp(const char *string1,
                    const char *string2);
void            ft_to_lower(char *string);
void            ft_to_upper(char *string);
char            *ft_strncpy(char *destination,
                    const char *source, size_t number_of_characters);
int             ft_strcpy_s(char *destination, size_t destination_size,
                    const char *source);
int             ft_strncpy_s(char *destination, size_t destination_size,
                    const char *source, size_t maximum_copy_length);
int             ft_strcat_s(char *destination, size_t destination_size,
                    const char *source);
int             ft_strncat_s(char *destination, size_t destination_size,
                    const char *source, size_t maximum_append_length);
char            *ft_span_dup(const char *buffer, size_t length);
char            *ft_strtok(char *string, const char *delimiters);
int             ft_locale_compare(const char *left, const char *right,
                    const char *locale_name);
void            *ft_memset(void *destination, int value, size_t number_of_bytes);
int             ft_isspace(int character);
char            *ft_strmapi(const char *string, char (*function)(unsigned int, char));
void            ft_striteri(char *string, void (*function)(unsigned int, char *));
size_t          ft_wstrlen(const wchar_t *string);
char16_t        *ft_utf8_to_utf16(const char *input, size_t input_length,
                    size_t *output_length_pointer);
char32_t        *ft_utf8_to_utf32(const char *input, size_t input_length,
                    size_t *output_length_pointer);

#endif
