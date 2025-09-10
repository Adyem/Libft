#ifndef PRINTF_HPP
#define PRINTF_HPP

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

int pf_printf(const char *format, ...) __attribute__((format(printf, 1, 2), hot));
int pf_printf_fd(int fd, const char *format, ...) __attribute__((format(printf, 2, 3), hot));
int pf_snprintf(char *string, size_t size, const char *format, ...) __attribute__((format(printf, 3, 4), hot));
int ft_vfprintf(FILE *stream, const char *format, va_list args);
int ft_fprintf(FILE *stream, const char *format, ...) __attribute__((format(printf, 2, 3), hot));

#endif
