#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <new>
#include <stdarg.h>

int32_t pf_snprintf(char *string, ft_size_t size, const char *format, ...)
{
    try
    {
        if (format == ft_nullptr || (string == ft_nullptr && size > 0))
        {
            if (string != ft_nullptr && size > 0)
                string[0] = '\0';
            return (-1);
        }
        va_list argument_list;
        va_start(argument_list, format);
        int32_t printed = pf_vsnprintf(string, size, format, argument_list);
        va_end(argument_list);
        return (printed);
    }
    catch (const std::bad_alloc &)
    {
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    catch (...)
    {
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
}
