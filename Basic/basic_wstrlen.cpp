#include <cstddef>
#include "basic.hpp"

size_t ft_wstrlen(const wchar_t *string)
{
    if (!string)
        return (0);
    size_t length = 0;
    while (string[length] != L'\0')
        ++length;
    return (length);
}
