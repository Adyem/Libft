#include "libft_config.hpp"

#if LIBFT_ENABLE_FILE_IO_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
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
        {
            if (open_error == EINVAL)
                ft_errno = FT_ERR_INVALID_ARGUMENT;
            else
                ft_errno = cmp_map_system_error_to_ft(open_error);
        }
        else
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (file_handle);
}
#endif

