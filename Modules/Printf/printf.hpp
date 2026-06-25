#ifndef PRINTF_HPP
#define PRINTF_HPP

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "../CPP_class/class_string.hpp"

typedef FILE *(*t_pf_tmpfile_function)(void);
typedef int32_t (*t_pf_fflush_function)(FILE *);
typedef int64_t (*t_pf_ftell_function)(FILE *);

int32_t pf_printf(const char *format, ...) __attribute__((format(printf, 1, 2), hot));
int32_t pf_printf_fd(int32_t file_descriptor, const char *format, ...)
    __attribute__((format(printf, 2, 3), hot));
int32_t pf_snprintf(char *string, ft_size_t size, const char *format, ...)
    __attribute__((format(printf, 3, 4), hot));
int32_t pf_vsnprintf(char *string, ft_size_t size, const char *format, va_list argument_list);
int32_t ft_vfprintf(FILE *stream, const char *format, va_list argument_list);
int32_t ft_fprintf(FILE *stream, const char *format, ...)
    __attribute__((format(printf, 2, 3), hot));
void pf_set_tmpfile_function(t_pf_tmpfile_function function);
void pf_reset_tmpfile_function(void);
void pf_set_fflush_function(t_pf_fflush_function function);
void pf_reset_fflush_function(void);
void pf_set_ftell_function(t_pf_ftell_function function);
void pf_reset_ftell_function(void);

typedef int32_t (*t_pf_custom_formatter)(va_list *argument_list, ft_string &output, void *context);

int32_t pf_register_custom_specifier(char specifier, t_pf_custom_formatter handler, void *context);
int32_t pf_unregister_custom_specifier(char specifier);
int32_t pf_enable_thread_safety(void);
int32_t pf_disable_thread_safety(void);

#endif
