#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char *ft_span_dup(const char *buffer, size_t length)
{
    char *duplicate;
    size_t index;

    ft_errno = FT_ER_SUCCESSS;
    if (buffer == ft_nullptr && length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    duplicate = static_cast<char *>(cma_malloc(length + 1));
    if (duplicate == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    index = 0;
    while (index < length)
    {
        duplicate[index] = buffer[index];
        index++;
    }
    duplicate[length] = '\0';
    return (duplicate);
}

ft_string ft_span_to_string(const char *buffer, size_t length)
{
    ft_string result;

    ft_errno = FT_ER_SUCCESSS;
    if (buffer == ft_nullptr && length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_string(FT_ERR_INVALID_ARGUMENT));
    }
    if (length == 0)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (result);
    }
    result.assign(buffer, length);
    if (result.get_error() != FT_ER_SUCCESSS)
    {
        return (ft_string(result.get_error()));
    }
    ft_errno = FT_ER_SUCCESSS;
    return (result);
}
