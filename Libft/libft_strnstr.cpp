#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strnstr(const char *haystack, const char *needle, size_t Length)
{
    size_t    haystackIndex = 0;
    size_t    matchIndex;
    char    *haystackPointer;
    size_t    needleLength;

    if (haystack == ft_nullptr || needle == ft_nullptr)
        return (ft_nullptr);
    haystackPointer = const_cast<char *>(haystack);
    needleLength = ft_strlen(needle);
    if (needleLength == 0 || haystack == needle)
        return (haystackPointer);
    while (haystackPointer[haystackIndex] != '\0' && haystackIndex < Length)
    {
        matchIndex = 0;
        while (haystackPointer[haystackIndex + matchIndex] != '\0'
            && needle[matchIndex] != '\0'
            && haystackPointer[haystackIndex + matchIndex] == needle[matchIndex]
            && haystackIndex + matchIndex < Length)
        {
            matchIndex++;
        }
        if (matchIndex == needleLength)
            return (haystackPointer + haystackIndex);

        haystackIndex++;
    }
    return (ft_nullptr);
}
