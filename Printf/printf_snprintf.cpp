#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <stdarg.h>

int pf_snprintf(char *string, size_t size, const char *format, ...)
{
    if (string == ft_nullptr || format == ft_nullptr)
        return (0);
    va_list args;
    va_start(args, format);
    int printed = pf_vsnprintf(string, size, format, args);
    va_end(args);
    return (printed);
}

