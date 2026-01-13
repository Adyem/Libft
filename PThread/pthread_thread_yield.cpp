#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int pt_thread_yield()
{
    int return_value;
    int error_code;

    return_value = cmp_thread_yield();
    if (return_value != 0)
    {
        error_code = ft_errno;
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}
