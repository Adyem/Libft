#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_strstr_result(int error_code, char *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

char    *ft_strstr(const char *haystack, const char *needle)
{
    size_t  haystack_length;
    char    *result;

    if (haystack == ft_nullptr || needle == ft_nullptr)
        return (report_strstr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    haystack_length = ft_strlen_size_t(haystack);
    result = ft_strnstr(haystack, needle, haystack_length);
    return (report_strstr_result(FT_ERR_SUCCESSS, result));
}
