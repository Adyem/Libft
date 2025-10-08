#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <cstdio>

int ft_fclose(FILE *stream)
{
    int close_result;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (EOF);
    }
    close_result = std::fclose(stream);
    if (close_result != 0)
    {
        int close_error;

        close_error = errno;
        if (close_error != 0)
            ft_errno = close_error + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (EOF);
    }
    ft_errno = ER_SUCCESS;
    return (FT_SUCCESS);
}

