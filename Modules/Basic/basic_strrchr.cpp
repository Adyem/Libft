#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

char    *ft_strrchr(const char *string, int32_t char_to_find)
{
    if (!string)
        return (ft_nullptr);

    ft_size_t index = ft_strlen_size_t(string);
    char target_char = static_cast<char>(char_to_find);
    while (FT_TRUE)
    {
        if (string[index] == target_char)
            return (const_cast<char *>(string + index));
        if (index == 0)
            break ;
        --index;
    }
    return (ft_nullptr);
}
