#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *ft_strnstr(const char *haystack, const char *needle, size_t maximum_length)
{
    size_t  haystack_index;
    size_t  match_index;
    char    *haystack_pointer;
    size_t  needle_length;

    if (needle == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (maximum_length == 0)
    {
        if (needle[0] == '\0')
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (const_cast<char *>(haystack));
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (ft_nullptr);
    }
    if (haystack == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    needle_length = ft_strlen_size_t(needle);
    haystack_pointer = const_cast<char *>(haystack);
    if (needle_length > maximum_length)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (ft_nullptr);
    }
    if (needle_length == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (haystack_pointer);
    }
    haystack_index = 0;
    while (haystack_pointer[haystack_index] != '\0' && haystack_index < maximum_length)
    {
        match_index = 0;
        while (haystack_pointer[haystack_index + match_index] != '\0'
            && match_index < needle_length
            && haystack_index + match_index < maximum_length
            && haystack_pointer[haystack_index + match_index] == needle[match_index])
        {
            match_index++;
        }
        if (match_index == needle_length)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (haystack_pointer + haystack_index);
        }
        haystack_index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (ft_nullptr);
}
