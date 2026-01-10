#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int pt_thread_yield()
{
    int return_value;

    return_value = cmp_thread_yield();
    if (return_value != 0)
    {
        ft_global_error_stack_push(ft_errno);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}
