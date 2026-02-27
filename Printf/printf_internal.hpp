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

size_t ft_strlen_printf(const char *string);
void ft_putchar_fd(const char character, int file_descriptor, size_t *count);
void ft_putstr_fd(const char *string, int file_descriptor, size_t *count);
void ft_putnbr_fd(long number, int file_descriptor, size_t *count);
void ft_putunsigned_fd(uintmax_t number, int file_descriptor, size_t *count);
void ft_puthex_fd(uintmax_t number, int file_descriptor, bool uppercase, size_t *count);
void ft_putoctal_fd(uintmax_t number, int file_descriptor, size_t *count);
void ft_putptr_fd(void *pointer, int file_descriptor, size_t *count);

void ft_putfloat_fd(double number, int file_descriptor, size_t *count, int precision);
void ft_putscientific_fd(double number, bool uppercase, int file_descriptor, size_t *count, int precision);
void ft_putgeneral_fd(double number, bool uppercase, int file_descriptor, size_t *count, int precision);
int pf_printf_fd_v(int file_descriptor, const char *format, va_list argument_list);
int pf_try_format_custom_specifier(char specifier, va_list *argument_list, ft_string &output, bool *handled);
void pf_write_ft_string_fd(const ft_string &output, int file_descriptor, size_t *count);
void pf_write_ft_string_stream(const ft_string &output, FILE *stream, size_t *count);
int pf_flush_stream(FILE *stream);
int pf_string_pop_last_error(const ft_string &);

#endif
