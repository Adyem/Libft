#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void    cma_set_thread_safety(bool enable)
{
    int error_code;

    (void)enable;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return ;
}
