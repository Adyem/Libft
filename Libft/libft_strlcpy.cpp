#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t    ft_strlcpy(char *destination, const char *source, size_t bufferSize)
{
    size_t    sourceLength;

    ft_errno = ER_SUCCESS;
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    sourceLength = 0;
    if (bufferSize == 0)
    {
        while (source[sourceLength] != '\0')
            sourceLength++;
        return (sourceLength);
    }
    while (sourceLength < bufferSize - 1 && source[sourceLength] != '\0')
    {
        destination[sourceLength] = source[sourceLength];
        sourceLength++;
    }
    if (sourceLength < bufferSize)
        destination[sourceLength] = '\0';
    while (source[sourceLength] != '\0')
        sourceLength++;
    return (sourceLength);
}
