#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strrchr(const char *string, int char_to_find)
{
    if (!string)
        return (ft_nullptr);

    size_t index = ft_strlen_size_t(string);
    char target_char = static_cast<char>(char_to_find);
    while (true)
    {
        if (string[index] == target_char)
            return (const_cast<char *>(string + index));
        if (index == 0)
            break ;
        --index;
    }
    return (ft_nullptr);
}
