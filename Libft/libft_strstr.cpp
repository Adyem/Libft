#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strstr(const char *haystack, const char *needle)
{
    size_t  haystack_length;

    if (!haystack || !needle)
        return (ft_nullptr);
    haystack_length = ft_strlen(haystack);
    return (ft_strnstr(haystack, needle, haystack_length));
}
