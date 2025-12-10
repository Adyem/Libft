#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *cma_substr(const char *source, unsigned int start, size_t length)
{
    size_t  source_length;
    size_t  start_index;
    size_t  available_length;
    size_t  index;
    char    *substring;

    if (!source)
        return (ft_nullptr);
    ft_errno = FT_ER_SUCCESSS;
    source_length = ft_strlen_size_t(source);
    if (ft_errno != FT_ER_SUCCESSS)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    start_index = static_cast<size_t>(start);
    if (start_index >= source_length)
        return (cma_strdup(""));
    available_length = source_length - start_index;
    if (length > available_length)
    {
        if (available_length >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            return (ft_nullptr);
        }
        length = available_length;
    }
    if (length >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    substring = static_cast<char *>(cma_malloc(length + 1));
    if (!substring)
        return (ft_nullptr);
    index = 0;
    while (index < length && source[start_index + index])
    {
        substring[index] = source[start_index + index];
        index++;
    }
    substring[index] = '\0';
    return (substring);
}
