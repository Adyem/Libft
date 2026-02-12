#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <errno.h>

int pt_thread_yield()
{
    int return_value;
    int error_code;

    return_value = cmp_thread_yield();
    if (return_value != 0)
    {
        error_code = ft_map_system_error(errno);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (return_value);
}
