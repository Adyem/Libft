#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <cstdio>

FILE *ft_fopen(const char *filename, const char *mode)
{
    FILE *file_handle;

    if (filename == ft_nullptr || mode == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_handle = std::fopen(filename, mode);
    if (file_handle == ft_nullptr)
    {
        int open_error;

        open_error = errno;
        if (open_error != 0)
            ft_errno = open_error + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (file_handle);
}

