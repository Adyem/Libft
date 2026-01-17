#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strstr(const char *haystack, const char *needle)
{
    size_t  haystack_length;
    char    *result;

    if (haystack == ft_nullptr || needle == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    haystack_length = ft_strlen_size_t(haystack);
    result = ft_strnstr(haystack, needle, haystack_length);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
