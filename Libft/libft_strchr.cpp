#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strchr(const char *string, int char_to_find)
{
    if (!string)
        return (ft_nullptr);
    char target_char = static_cast<char>(char_to_find);
    while (*string)
    {
        if (*string == target_char)
            return (const_cast<char *>(string));
        ++string;
    }
    if (target_char == '\0')
        return (const_cast<char *>(string));
    return (ft_nullptr);
}
