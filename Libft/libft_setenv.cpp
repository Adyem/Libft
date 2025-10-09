#include "libft_config.hpp"
#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
#endif
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "libft_environment_lock.hpp"
#include <cstdlib>
#include <cerrno>

int ft_setenv(const char *name, const char *value, int overwrite)
{
    int result;
    int stored_errno;

    ft_errno = ER_SUCCESS;
    if (name == ft_nullptr || value == ft_nullptr || *name == '\0' || ft_strchr(name, '=') != ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (ft_environment_lock() != 0)
        return (-1);
    result = cmp_setenv(name, value, overwrite);
    if (result != 0)
    {
#if defined(_WIN32) || defined(_WIN64)
        int saved_errno;
        int last_error;
        int socket_error;

        saved_errno = errno;
        last_error = GetLastError();
        socket_error = WSAGetLastError();
        if (result > 0)
            ft_errno = result + ERRNO_OFFSET;
        else if (last_error != 0)
            ft_errno = last_error + ERRNO_OFFSET;
        else if (socket_error != 0)
            ft_errno = socket_error + ERRNO_OFFSET;
        else if (saved_errno != 0)
            ft_errno = saved_errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
#else
        int saved_errno;

        saved_errno = errno;
        if (saved_errno != 0)
            ft_errno = saved_errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
#endif
    }
    stored_errno = ft_errno;
    if (ft_environment_unlock() != 0)
    {
        if (result != 0)
            ft_errno = stored_errno;
        if (result == 0)
            return (-1);
        return (result);
    }
    ft_errno = stored_errno;
    return (result);
}
#endif
