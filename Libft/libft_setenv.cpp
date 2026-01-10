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

static int report_env_error(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int ft_setenv(const char *name, const char *value, int overwrite)
{
    int result;
    char *invalid_character;
    int error_code;
    int unlock_error;

    if (name == ft_nullptr || value == ft_nullptr || *name == '\0')
        return (report_env_error(FT_ERR_INVALID_ARGUMENT, -1));
    invalid_character = ft_strchr(name, '=');
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        return (report_env_error(error_code, -1));
    }
    if (invalid_character != ft_nullptr)
        return (report_env_error(FT_ERR_INVALID_ARGUMENT, -1));
    if (ft_environment_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (report_env_error(error_code, -1));
    }
    ft_global_error_stack_pop_newest();
    error_code = FT_ERR_SUCCESSS;
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
            error_code = ft_map_system_error(result);
        else if (last_error != 0)
            error_code = ft_map_system_error(last_error);
        else if (socket_error != 0)
            error_code = ft_map_system_error(socket_error);
        else if (saved_errno != 0)
            error_code = ft_map_system_error(saved_errno);
        else
            error_code = FT_ERR_TERMINATED;
#else
        int saved_errno;

        saved_errno = errno;
        if (saved_errno != 0)
            error_code = ft_map_system_error(saved_errno);
        else
            error_code = FT_ERR_TERMINATED;
#endif
    }
    if (ft_environment_unlock() != 0)
    {
        unlock_error = ft_global_error_stack_pop_newest();
        if (unlock_error == FT_ERR_SUCCESSS)
            unlock_error = FT_ERR_MUTEX_NOT_OWNER;
        if (result == 0)
        {
            return (report_env_error(unlock_error, -1));
        }
        return (report_env_error(error_code, result));
    }
    ft_global_error_stack_pop_newest();
    if (result != 0)
    {
        return (report_env_error(error_code, result));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
#endif
