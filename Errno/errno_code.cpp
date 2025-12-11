#include "errno.hpp"

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = FT_ERR_SUCCESSS;

    return (thread_errno);
}
