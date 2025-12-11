#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <stdarg.h>

int pf_snprintf(char *string, size_t size, const char *format, ...)
{
    if (format == ft_nullptr || (string == ft_nullptr && size > 0))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    va_list args;
    va_start(args, format);
    int printed = pf_vsnprintf(string, size, format, args);
    va_end(args);
    if (printed < 0)
        return (printed);
    ft_errno = FT_ERR_SUCCESSS;
    return (printed);
}

