#include "printf.hpp"
#include "printf_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdarg>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

int pf_printf_fd(int fd, const char *format, ...)
{
    va_list args;
    int        printed_chars;
    int        error_code;

    if (!format)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(fd, format, args);
    va_end(args);
    error_code = ft_global_error_stack_drop_last_error();
    if (printed_chars < 0)
    {
        if (error_code != FT_ERR_SUCCESS)
            ft_global_error_stack_push(error_code);
        return (printed_chars);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (printed_chars);
}

int pf_printf(const char *format, ...)
{
    va_list    args;
    int        printed_chars;
    int        error_code;

    if (!format)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(1, format, args);
    va_end(args);
    error_code = ft_global_error_stack_drop_last_error();
    if (printed_chars < 0)
    {
        if (error_code != FT_ERR_SUCCESS)
            ft_global_error_stack_push(error_code);
        return (printed_chars);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (printed_chars);
}
