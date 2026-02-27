#include "printf.hpp"
#include "printf_internal.hpp"
#include <cstdarg>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

int pf_printf_fd(int file_descriptor, const char *format, ...)
{
    va_list argument_list;
    int printed_chars;

    if (!format)
        return (-1);
    va_start(argument_list, format);
    printed_chars = pf_printf_fd_v(file_descriptor, format, argument_list);
    va_end(argument_list);
    return (printed_chars);
}

int pf_printf(const char *format, ...)
{
    va_list argument_list;
    int printed_chars;

    if (!format)
        return (-1);
    va_start(argument_list, format);
    printed_chars = pf_printf_fd_v(1, format, argument_list);
    va_end(argument_list);
    return (printed_chars);
}
