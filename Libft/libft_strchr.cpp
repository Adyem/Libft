#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strchr(const char *string, int char_to_find)
{
    if (!string)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char target_char = static_cast<char>(char_to_find);
    while (*string)
    {
        if (*string == target_char)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (const_cast<char *>(string));
        }
        ++string;
    }
    if (target_char == '\0')
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (const_cast<char *>(string));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (ft_nullptr);
}
