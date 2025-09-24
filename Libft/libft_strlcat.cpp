#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t    ft_strlcat(char *destination, const char *source, size_t bufferSize)
{
    size_t    destLength = 0;
    size_t    sourceIndex = 0;

    ft_errno = ER_SUCCESS;
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    while (destination[destLength] && destLength < bufferSize)
        destLength++;
    while (source[sourceIndex] && (destLength + sourceIndex + 1) < bufferSize)
    {
        destination[destLength + sourceIndex] = source[sourceIndex];
        sourceIndex++;
    }
    if (destLength < bufferSize)
        destination[destLength + sourceIndex] = '\0';
    return (destLength + ft_strlen(source));
}
