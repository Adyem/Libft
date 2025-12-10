#include "errno.hpp"

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = FT_ER_SUCCESSS;

    return (thread_errno);
}
