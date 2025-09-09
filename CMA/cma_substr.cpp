#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *cma_substr(const char *source, unsigned int start, size_t length)
{
    size_t  source_length;
    size_t  index;
    char    *substring;

    if (!source)
        return (ft_nullptr);
    source_length = ft_strlen(source);
    if (start >= source_length)
        return (cma_strdup(""));
    if (length > source_length - start)
        length = source_length - start;
    substring = static_cast<char *>(cma_malloc(length + 1));
    if (!substring)
        return (ft_nullptr);
    index = 0;
    while (index < length && source[start + index])
    {
        substring[index] = source[start + index];
        index++;
    }
    substring[index] = '\0';
    return (substring);
}
