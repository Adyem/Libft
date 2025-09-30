#include "printf.hpp"
#include "printf_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
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

    if (format == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(fd, format, args);
    va_end(args);
    if (printed_chars >= 0)
        ft_errno = ER_SUCCESS;
    return (printed_chars);
}

int pf_printf(const char *format, ...)
{
    va_list    args;
    int        printed_chars;

    if (format == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(1, format, args);
    va_end(args);
    if (printed_chars >= 0)
        ft_errno = ER_SUCCESS;
    return (printed_chars);
}
