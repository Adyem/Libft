#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <stdarg.h>
#include <stdio.h>

int pf_vsnprintf(char *string, size_t size, const char *format, va_list args)
{
    if (string == ft_nullptr || format == ft_nullptr)
        return (0);
    FILE *stream = tmpfile();
    if (stream == ft_nullptr)
    {
        if (size > 0)
            string[0] = '\0';
        return (0);
    }
    va_list copy;
    va_copy(copy, args);
    int printed = ft_vfprintf(stream, format, copy);
    va_end(copy);
    if (printed < 0)
    {
        fclose(stream);
        if (size > 0)
            string[0] = '\0';
        return (printed);
    }
    int flush_status = fflush(stream);
    if (flush_status != 0)
    {
        fclose(stream);
        if (size > 0)
            string[0] = '\0';
        return (0);
    }
    long position = ftell(stream);
    if (position < 0)
    {
        fclose(stream);
        if (size > 0)
            string[0] = '\0';
        return (0);
    }
    rewind(stream);
    if (size > 0)
    {
        size_t copy_length = static_cast<size_t>(position);
        if (copy_length >= size)
            copy_length = size - 1;
        size_t read_bytes = 0;
        if (copy_length > 0)
            read_bytes = fread(string, 1, copy_length, stream);
        string[read_bytes] = '\0';
    }
    fclose(stream);
    return (printed);
}

