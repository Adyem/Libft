#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_strrchr_result(int error_code, const char *result)
{
    ft_global_error_stack_push(error_code);
    return (const_cast<char *>(result));
}

char    *ft_strrchr(const char *string, int char_to_find)
{
    if (!string)
    {
        return (report_strrchr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    }
    size_t string_length = ft_strlen_size_t(string);
    char target_char = static_cast<char>(char_to_find);
    while (string_length > 0)
    {
        if (string[string_length] == target_char)
        {
            return (report_strrchr_result(FT_ERR_SUCCESSS,
                string + string_length));
        }
        --string_length;
    }
    if (string[string_length] == target_char)
    {
        return (report_strrchr_result(FT_ERR_SUCCESSS,
            string + string_length));
    }
    return (report_strrchr_result(FT_ERR_SUCCESSS, ft_nullptr));
}
