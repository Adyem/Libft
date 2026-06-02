#include "printf.hpp"
#include "printf_internal.hpp"
#include <cstdarg>
#include <unistd.h>
#include <new>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t pf_printf_fd(int32_t file_descriptor, const char *format, ...)
{
    try
    {
        va_list argument_list;
        int32_t printed_chars;

        if (!format)
            return (-1);
        va_start(argument_list, format);
        printed_chars = pf_printf_fd_v(file_descriptor, format, argument_list);
        va_end(argument_list);
        return (printed_chars);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    catch (...)
    {
        return (-1);
    }
}

int32_t pf_printf(const char *format, ...)
{
    try
    {
        va_list argument_list;
        int32_t printed_chars;

        if (!format)
            return (-1);
        va_start(argument_list, format);
        printed_chars = pf_printf_fd_v(1, format, argument_list);
        va_end(argument_list);
        return (printed_chars);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    catch (...)
    {
        return (-1);
    }
}
