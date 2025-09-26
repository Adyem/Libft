#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strrchr(const char *string, int char_to_find)
{
    if (!string)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    size_t string_length = ft_strlen_size_t(string);
    char target_char = static_cast<char>(char_to_find);
    while (string_length > 0)
    {
        if (string[string_length] == target_char)
            return (const_cast<char *>(string) + string_length);
        --string_length;
    }
    if (string[string_length] == target_char)
        return (const_cast<char *>(string) + string_length);
    return (ft_nullptr);
}
