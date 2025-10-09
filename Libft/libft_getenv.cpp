#include "libft_config.hpp"
#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "libft_environment_lock.hpp"
#include <cstdlib>

char    *ft_getenv(const char *name)
{
    char    *value;
    int     stored_errno;

    ft_errno = ER_SUCCESS;
    if (name == ft_nullptr || *name == '\0')
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (ft_environment_lock() != 0)
        return (ft_nullptr);
    value = getenv(name);
    stored_errno = ft_errno;
    if (ft_environment_unlock() != 0)
        return (ft_nullptr);
    ft_errno = stored_errno;
    return (value);
}
#endif
