#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

static pt_mutex g_env_mutex;

char    *su_getenv(const char *name)
{
    char    *result;

    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_nullptr);
    result = ft_getenv(name);
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (ft_nullptr);
    return (result);
}

int su_setenv(const char *name, const char *value, int overwrite)
{
    int result;

    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    result = ft_setenv(name, value, overwrite);
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    return (result);
}

int su_putenv(char *string)
{
    int result;
    int cmp_error_code;

    if (string == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    cmp_error_code = ER_SUCCESS;
    result = cmp_putenv(string);
    if (result != 0)
        cmp_error_code = ft_errno;
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    if (result == 0)
        ft_errno = ER_SUCCESS;
    else
        ft_errno = cmp_error_code;
    return (result);
}
