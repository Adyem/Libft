#include "libft_config.hpp"

#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "libft_environment_lock.hpp"
#include <cstdlib>

static char *report_env_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return (ft_nullptr);
}

char    *ft_getenv(const char *name)
{
    char    *value;
    int     error_code;

    if (name == ft_nullptr || *name == '\0')
        return (report_env_error(FT_ERR_INVALID_ARGUMENT));
    if (ft_environment_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (report_env_error(error_code));
    }
    ft_global_error_stack_pop_newest();
    value = getenv(name);
    if (ft_environment_unlock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_MUTEX_NOT_OWNER;
        return (report_env_error(error_code));
    }
    ft_global_error_stack_pop_newest();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}
#endif
