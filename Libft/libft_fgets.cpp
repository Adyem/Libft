#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

char *ft_fgets(char *string, int size, FILE *stream)
{
    char *result_string;

    if (string == ft_nullptr || stream == ft_nullptr || size <= 0)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    result_string = std::fgets(string, size, stream);
    if (result_string == ft_nullptr)
    {
        if (std::ferror(stream) != 0)
            ft_errno = FILE_INVALID_FD;
        else
            ft_errno = ER_SUCCESS;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (result_string);
}

