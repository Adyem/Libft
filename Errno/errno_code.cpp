#include "errno.hpp"

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = FT_ERR_SUCCESSS;

    return (thread_errno);
}

int &ft_sys_errno_reference(void)
{
    static thread_local int thread_sys_errno = FT_SYS_ERR_SUCCESS;

    return (thread_sys_errno);
}
