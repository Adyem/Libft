#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

char *ft_strchr(const char *string, int char_to_find)
{
    if (!string)
        return (ft_nullptr);
    const unsigned char *s = reinterpret_cast<const unsigned char*>(string);
    unsigned char c = static_cast<unsigned char>(char_to_find);
    while (*s)
    {
        if (*s == c)
            return (reinterpret_cast<char*>(const_cast<unsigned char*>(s)));
        ++s;
    }
    if (c == 0)
        return (reinterpret_cast<char*>(const_cast<unsigned char*>(s)));
    return (ft_nullptr);
}
