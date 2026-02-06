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
    char *invalid_character;
    int error_code;
    int unlock_error;

    if (name == ft_nullptr || value == ft_nullptr || *name == '\0')
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    invalid_character = ft_strchr(name, '=');
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (invalid_character != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (ft_environment_lock() != 0)
    {
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_MUTEX_ALREADY_LOCKED;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    error_code = FT_ERR_SUCCESSS;
    result = cmp_setenv(name, value, overwrite);
    error_code = cmp_last_error();
    if (ft_environment_unlock() != 0)
    {
        unlock_error = ft_global_error_stack_drop_last_error();
        if (unlock_error == FT_ERR_SUCCESSS)
            unlock_error = FT_ERR_MUTEX_NOT_OWNER;
        if (result == 0)
        {
            ft_global_error_stack_push(unlock_error);
            return (-1);
        }
        ft_global_error_stack_push(error_code);
        return (result);
    }
    ft_global_error_stack_drop_last_error();
    if (result != 0)
    {
        ft_global_error_stack_push(error_code);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
#endif
