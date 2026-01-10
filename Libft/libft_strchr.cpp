#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_strchr_result(int error_code, const char *result)
{
    ft_global_error_stack_push(error_code);
    return (const_cast<char *>(result));
}

char    *ft_strchr(const char *string, int char_to_find)
{
    if (!string)
    {
        return (report_strchr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    }
    char target_char = static_cast<char>(char_to_find);
    while (*string)
    {
        if (*string == target_char)
        {
            return (report_strchr_result(FT_ERR_SUCCESSS, string));
        }
        ++string;
    }
    if (target_char == '\0')
    {
        return (report_strchr_result(FT_ERR_SUCCESSS, string));
    }
    return (report_strchr_result(FT_ERR_SUCCESSS, ft_nullptr));
}
