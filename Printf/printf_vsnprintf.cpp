#include "printf.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int pf_vsnprintf(char *string, size_t size, const char *format, va_list args)
{
    if (string == ft_nullptr || format == ft_nullptr)
        return (0);
    char *buffer = ft_nullptr;
    size_t buffer_size = 0;
    FILE *stream = open_memstream(&buffer, &buffer_size);
    if (stream == ft_nullptr)
        return (0);
    va_list copy;
    va_copy(copy, args);
    int printed = ft_vfprintf(stream, format, copy);
    va_end(copy);
    fclose(stream);
    if (buffer != ft_nullptr && size > 0)
        ft_strlcpy(string, buffer, size);
    free(buffer);
    return (printed);
}

