#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>

static pt_mutex g_env_mutex;

char    *su_getenv(const char *name)
{
    char    *result;

    if (g_env_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    result = ft_getenv(name);
    if (g_env_mutex.unlock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    return (result);
}

int su_setenv(const char *name, const char *value, int overwrite)
{
    int result;

    if (g_env_mutex.lock(THREAD_ID) != SUCCES)
        return (-1);
    result = ft_setenv(name, value, overwrite);
    if (g_env_mutex.unlock(THREAD_ID) != SUCCES)
        return (-1);
    return (result);
}

int su_putenv(char *string)
{
    int result;

    if (string == ft_nullptr)
        return (-1);
    if (g_env_mutex.lock(THREAD_ID) != SUCCES)
        return (-1);
#if defined(_WIN32) || defined(_WIN64)
    result = _putenv(string);
#else
    result = putenv(string);
#endif
    if (g_env_mutex.unlock(THREAD_ID) != SUCCES)
        return (-1);
    return (result);
}
