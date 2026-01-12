#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

unsigned int    su_get_cpu_count(void)
{
    unsigned int result;
    int error_code;

    result = cmp_get_cpu_count();
    if (result == 0)
    {
        error_code = ft_errno;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_TERMINATED;
        ft_global_error_stack_push(error_code);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

unsigned long long su_get_total_memory(void)
{
    unsigned long long result;
    int error_code;

    error_code = cmp_get_total_memory(&result);
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
