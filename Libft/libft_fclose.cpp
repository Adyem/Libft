#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

int ft_fclose(FILE *stream)
{
    int close_result;

    if (stream == ft_nullptr)
    {
        ft_errno = FILE_INVALID_FD;
        return (EOF);
    }
    close_result = std::fclose(stream);
    if (close_result != 0)
    {
        ft_errno = FILE_INVALID_FD;
        return (EOF);
    }
    ft_errno = ER_SUCCESS;
    return (FT_SUCCESS);
}

