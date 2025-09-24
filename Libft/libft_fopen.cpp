#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

FILE *ft_fopen(const char *filename, const char *mode)
{
    FILE *file_handle;

    if (filename == ft_nullptr || mode == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    file_handle = std::fopen(filename, mode);
    if (file_handle == ft_nullptr)
    {
        ft_errno = FILE_INVALID_FD;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (file_handle);
}

