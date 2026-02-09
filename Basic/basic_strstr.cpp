#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strstr(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return (ft_nullptr);
    size_t index = 0;
    size_t needle_length = ft_strlen_size_t(needle);
    if (needle_length == 0)
        return (const_cast<char *>(haystack));
    while (haystack[index] != '\0')
    {
        size_t match_index = 0;
        while (needle[match_index] != '\0'
               && haystack[index + match_index] == needle[match_index])
        {
            ++match_index;
        }
        if (match_index == needle_length)
            return (const_cast<char *>(haystack + index));
        ++index;
    }
    return (ft_nullptr);
}
