#ifndef LIBFT_HPP
# define LIBFT_HPP

#define SUCCES 0
#define FAILURE 1

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "../PThread/mutex.hpp"

size_t             ft_strlen_size_t(const char *string);
int                ft_strlen(const char *string);
char            *ft_strchr(const char *string, int char_to_find);
int                ft_atoi(const char *string);
void            ft_bzero(void *string, size_t size);
void            *ft_memchr(const void *pointer, int character, size_t size);
void            *ft_memcpy(void* destination, const void* source, size_t num);
void            *ft_memmove(void *destination, const void *source, size_t size);
void            *ft_memdup(const void *source, size_t size);
char            *ft_strchr(const char *string, int char_to_find);
size_t            ft_strlcat(char *destination, const char *source, size_t bufferSize);
size_t            ft_strlcpy(char *destination, const char *source, size_t bufferSize);
char            *ft_strrchr(const char *string, int char_to_find);
char            *ft_strnstr(const char *haystack, const char *needle, size_t length);
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
void             *ft_memset(void *destination, int value, size_t number_of_bytes);
int             ft_isspace(int character);
int             ft_abs(int number);
void            ft_swap(int *a, int *b);
int             ft_clamp(int value, int min, int max);
double          ft_pow(double base, int exponent);
double          ft_sqrt(double number);
double          ft_exp(double x);
char            *ft_strjoin_multiple(int count, ...);
char            *ft_strmapi(const char *string, char (*function)(unsigned int, char));
void            ft_striteri(char *string, void (*function)(unsigned int, char *));
char            *ft_getenv(const char *name);
int             ft_setenv(const char *name, const char *value, int overwrite);
int             ft_unsetenv(const char *name);

FILE            *ft_fopen(const char *filename, const char *mode);
int             ft_fclose(FILE *stream);
size_t          ft_fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t          ft_fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int             ft_fseek(FILE *stream, long offset, int origin);
long            ft_ftell(FILE *stream);
long            ft_time_ms(void);
char            *ft_time_format(char *buffer, size_t buffer_size);

#endif
