#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strnstr(const char *haystack, const char *needle, size_t maximum_length)
{
    if (needle == ft_nullptr || haystack == ft_nullptr)
        return (ft_nullptr);

    size_t needle_length = ft_strlen_size_t(needle);
    if (needle_length == 0)
        return (const_cast<char *>(haystack));

    size_t haystack_index = 0;
    while (haystack_index < maximum_length && haystack[haystack_index] != '\0')
    {
        size_t match_index = 0;
        while (match_index < needle_length
            && haystack_index + match_index < maximum_length
            && haystack[haystack_index + match_index] == needle[match_index])
        {
            ++match_index;
        }
        if (match_index == needle_length)
            return (const_cast<char *>(haystack + haystack_index));
        ++haystack_index;
    }
    return (ft_nullptr);
}
