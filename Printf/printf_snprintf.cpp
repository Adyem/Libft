#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <stdarg.h>

int pf_snprintf(char *string, size_t size, const char *format, ...)
{
    if (format == ft_nullptr || (string == ft_nullptr && size > 0))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    va_list args;
    va_start(args, format);
    int printed = pf_vsnprintf(string, size, format, args);
    va_end(args);
    if (printed < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_IO;
        ft_global_error_stack_push(error_code);
        return (printed);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (printed);
}
