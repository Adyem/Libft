#ifndef PRINTF_INTERNAL_HPP
# define PRINTF_INTERNAL_HPP

#include "printf.hpp"
#include <cstdarg>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <cstdio>

typedef enum
{
    LEN_NONE,
    LEN_L,
    LEN_Z
} LengthModifier;

size_t ft_strlen_printf(const char *string);
void ft_putchar_fd(const char character, int fd, size_t *count);
void ft_putstr_fd(const char *string, int fd, size_t *count);
void ft_putnbr_fd_recursive(long number, int fd, size_t *count);
void ft_putnbr_fd(long number, int fd, size_t *count);
void ft_putunsigned_fd_recursive(uintmax_t number, int fd, size_t *count);
void ft_putunsigned_fd(uintmax_t number, int fd, size_t *count);
void ft_puthex_fd_recursive(uintmax_t number, int fd, bool uppercase, size_t *count);
void ft_puthex_fd(uintmax_t number, int fd, bool uppercase, size_t *count);
void ft_putoctal_fd_recursive(uintmax_t number, int fd, size_t *count);
void ft_putoctal_fd(uintmax_t number, int fd, size_t *count);
void ft_putptr_fd(void *pointer, int fd, size_t *count);

void ft_putfloat_fd(double number, int fd, size_t *count, int precision);
void ft_putscientific_fd(double number, bool uppercase, int fd, size_t *count, int precision);
void ft_putgeneral_fd(double number, bool uppercase, int fd, size_t *count, int precision);
int pf_printf_fd_v(int fd, const char *format, va_list args);
int pf_try_format_custom_specifier(char specifier, va_list *args, ft_string &output);
void pf_write_ft_string_fd(const ft_string &output, int fd, size_t *count);
void pf_write_ft_string_stream(const ft_string &output, FILE *stream, size_t *count);

#endif
