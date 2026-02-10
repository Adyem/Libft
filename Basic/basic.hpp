#ifndef LIBFT_HPP
# define LIBFT_HPP

#include <cstdint>
#define FT_SUCCESS 0
#define FT_FAILURE 1

#ifndef FT_TYPES_HPP
# define FT_TYPES_HPP

typedef uint64_t ft_size_t;

#endif

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdarg.h>
 #include "limits.hpp"

 class ft_string;
static constexpr ft_size_t ft_strlen_raw(const char *string)
{
    const char *string_pointer = string;

    while (*string_pointer)
        ++string_pointer;
    return (static_cast<ft_size_t>(string_pointer - string));
}

constexpr ft_size_t ft_strlen_size_t(const char *string)
{
    if (!string)
        return (0);
    return (ft_strlen_raw(string));
}

constexpr int ft_strlen(const char *string)
{
    if (!string)
        return (0);
    ft_size_t length = ft_strlen_raw(string);
    if (length > static_cast<ft_size_t>(FT_INT_MAX))
        return (FT_INT_MAX);
    return (static_cast<int>(length));
}

char            *ft_strchr(const char *string, int char_to_find);
int32_t         ft_atoi(const char *string);
int32_t         ft_validate_int(const char *input);
void            ft_bzero(void *string, ft_size_t size);
void            *ft_memchr(const void *pointer, int32_t character, ft_size_t size);
void            *ft_memcpy(void *destination, const void *source, ft_size_t size);
int32_t         ft_memcpy_s(void *destination, ft_size_t destination_size,
                    const void *source, ft_size_t number_of_bytes);
void            *ft_memmove(void *destination, const void *source, ft_size_t size);
int32_t         ft_memmove_s(void *destination, ft_size_t destination_size,
                    const void *source, ft_size_t number_of_bytes);
ft_size_t          ft_strlcat(char *destination, const char *source, ft_size_t buffer_size);
ft_size_t          ft_strlcpy(char *destination, const char *source, ft_size_t buffer_size);
ft_size_t          ft_strnlen(const char *string, ft_size_t maximum_length);
char            *ft_strrchr(const char *string, int32_t char_to_find);
char            *ft_strnstr(const char *haystack, const char *needle,
                    ft_size_t maximum_length);
char            *ft_strstr(const char *haystack, const char *needle);
int32_t         ft_strncmp(const char *string_1, const char *string_2,
                    ft_size_t maximum_length);
int32_t         ft_memcmp(const void *pointer1, const void *pointer2, ft_size_t size);
int32_t         ft_isdigit(int32_t character);
int32_t         ft_isalpha(int32_t character);
int32_t         ft_isalnum(int32_t character);
int32_t         ft_isprint(int32_t character);
int32_t         ft_islower(int32_t character);
int32_t         ft_isupper(int32_t character);
int64_t         ft_atol(const char *string);
int64_t         ft_strtol(const char *input_string, char **end_pointer,
                    int32_t numeric_base);
uint64_t        ft_strtoul(const char *input_string,
                    char **end_pointer, int32_t numeric_base);
int32_t         ft_strcmp(const char *string1,
                    const char *string2);
void            ft_to_lower(char *string);
void            ft_to_upper(char *string);
char            *ft_strncpy(char *destination,
                    const char *source, ft_size_t number_of_characters);
int32_t         ft_strcpy_s(char *destination, ft_size_t destination_size,
                    const char *source);
int32_t         ft_strncpy_s(char *destination, ft_size_t destination_size,
                    const char *source, ft_size_t maximum_copy_length);
int32_t         ft_strcat_s(char *destination, ft_size_t destination_size,
                    const char *source);
int32_t         ft_strncat_s(char *destination, ft_size_t destination_size,
                    const char *source, ft_size_t maximum_append_length);
char            *ft_span_dup(const char *buffer, ft_size_t length);
char            *ft_strtok(char *string, const char *delimiters);
int32_t         ft_locale_compare(const char *left, const char *right,
                    const char *locale_name);
void            *ft_memset(void *destination, int value, ft_size_t number_of_bytes);
int32_t         ft_isspace(int32_t character);
char            *ft_strmapi(const char *string, char (*function)(unsigned int, char));
void            ft_striteri(char *string, void (*function)(unsigned int, char *));
ft_size_t          ft_wstrlen(const wchar_t *string);
char16_t        *ft_utf8_to_utf16(const char *input, ft_size_t input_length,
                    ft_size_t *output_length_pointer);
char32_t        *ft_utf8_to_utf32(const char *input, ft_size_t input_length,
                    ft_size_t *output_length_pointer);

#endif
