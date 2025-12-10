#include "libft_config.hpp"

#if LIBFT_ENABLE_FILE_IO_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
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
            ft_errno = cmp_map_system_error_to_ft(close_error);
        else
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (EOF);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (FT_SUCCESS);
}
#endif

