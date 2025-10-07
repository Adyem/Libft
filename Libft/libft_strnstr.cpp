#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strnstr(const char *haystack, const char *needle, size_t max_length)
{
    size_t  haystack_index;
    size_t  match_index;
    char    *haystack_pointer;
    size_t  needle_length;

    ft_errno = ER_SUCCESS;
    if (needle == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    if (max_length == 0)
    {
        ft_errno = ER_SUCCESS;
        if (needle[0] == '\0')
            return (const_cast<char *>(haystack));
        return (ft_nullptr);
    }
    if (haystack == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    needle_length = ft_strlen_size_t(needle);
    if (ft_errno != ER_SUCCESS)
        return (ft_nullptr);
    haystack_pointer = const_cast<char *>(haystack);
    if (needle_length > max_length)
        return (ft_nullptr);
    if (needle_length == 0)
        return (haystack_pointer);
    haystack_index = 0;
    while (haystack_pointer[haystack_index] != '\0' && haystack_index < max_length)
    {
        match_index = 0;
        while (haystack_pointer[haystack_index + match_index] != '\0'
            && match_index < needle_length
            && haystack_index + match_index < max_length
            && haystack_pointer[haystack_index + match_index] == needle[match_index])
        {
            match_index++;
        }
        if (match_index == needle_length)
            return (haystack_pointer + haystack_index);
        haystack_index++;
    }
    return (ft_nullptr);
}
