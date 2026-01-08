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

    if (!format)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(fd, format, args);
    va_end(args);
    if (printed_chars < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        ft_global_error_stack_push(error_code);
        return (printed_chars);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (printed_chars);
}

int pf_printf(const char *format, ...)
{
    va_list    args;
    int        printed_chars;

    if (!format)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(1, format, args);
    va_end(args);
    if (printed_chars < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        ft_global_error_stack_push(error_code);
        return (printed_chars);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (printed_chars);
}
