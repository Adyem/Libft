#include <stddef.h>
#include <stdbool.h>
#include "../CPP_class/class_nullptr.hpp"
#include "CMA.hpp"

char    *cma_strdup(const char *string)
{
    int        length;
    char    *new_string;
    int        index;

    if (!string)
        return (ft_nullptr);
    length = 0;
    while (string[length])
        length++;
    length++;
    new_string = static_cast<char *>(cma_malloc(length));
    if (!new_string)
        return (ft_nullptr);
    index = 0;
    while (index < length)
    {
        new_string[index] = string[index];
        index++;
    }
    return (new_string);
}
