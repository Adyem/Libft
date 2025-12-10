#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strstr(const char *haystack, const char *needle)
{
    size_t  haystack_length;

    ft_errno = FT_ER_SUCCESSS;
    if (haystack == ft_nullptr || needle == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    haystack_length = ft_strlen_size_t(haystack);
    if (ft_errno != FT_ER_SUCCESSS)
        return (ft_nullptr);
    return (ft_strnstr(haystack, needle, haystack_length));
}
