#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_strnstr_result(int error_code, char *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

char    *ft_strnstr(const char *haystack, const char *needle, size_t max_length)
{
    size_t  haystack_index;
    size_t  match_index;
    char    *haystack_pointer;
    size_t  needle_length;

    if (needle == ft_nullptr)
    {
        return (report_strnstr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    }
    if (max_length == 0)
    {
        if (needle[0] == '\0')
        {
            return (report_strnstr_result(FT_ERR_SUCCESSS,
                const_cast<char *>(haystack)));
        }
        return (report_strnstr_result(FT_ERR_SUCCESSS, ft_nullptr));
    }
    if (haystack == ft_nullptr)
    {
        return (report_strnstr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    }
    needle_length = ft_strlen_size_t(needle);
    haystack_pointer = const_cast<char *>(haystack);
    if (needle_length > max_length)
    {
        return (report_strnstr_result(FT_ERR_SUCCESSS, ft_nullptr));
    }
    if (needle_length == 0)
    {
        return (report_strnstr_result(FT_ERR_SUCCESSS, haystack_pointer));
    }
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
        {
            return (report_strnstr_result(FT_ERR_SUCCESSS,
                haystack_pointer + haystack_index));
        }
        haystack_index++;
    }
    return (report_strnstr_result(FT_ERR_SUCCESSS, ft_nullptr));
}
