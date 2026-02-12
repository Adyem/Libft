#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <stdarg.h>

int pf_snprintf(char *string, size_t size, const char *format, ...)
{
    int error_code;

    if (format == ft_nullptr || (string == ft_nullptr && size > 0))
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    va_list args;
    va_start(args, format);
    int printed = pf_vsnprintf(string, size, format, args);
    va_end(args);
    error_code = ft_global_error_stack_drop_last_error();
    if (printed < 0)
    {
        if (error_code != FT_ERR_SUCCESS)
            ft_global_error_stack_push(error_code);
        return (printed);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (printed);
}
