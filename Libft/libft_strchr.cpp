#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strchr(const char *string, int char_to_find)
{
    ft_errno = FT_ER_SUCCESSS;
    if (!string)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
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
