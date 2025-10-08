#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>
#include <cerrno>

char *ft_fgets(char *string, int size, FILE *stream)
{
    char *result_string;

    if (string == ft_nullptr || stream == ft_nullptr || size <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    result_string = std::fgets(string, size, stream);
    if (result_string == ft_nullptr)
    {
        if (std::ferror(stream) != 0)
        {
            int saved_errno;

            saved_errno = errno;
            ft_errno = saved_errno != 0 ? saved_errno + ERRNO_OFFSET : FT_ERR_INVALID_HANDLE;
            return (ft_nullptr);
        }
        if (std::feof(stream) != 0)
        {
            ft_errno = FT_ERR_END_OF_FILE;
            return (ft_nullptr);
        }
        ft_errno = ER_SUCCESS;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (result_string);
}

