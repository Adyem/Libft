#include "errno.hpp"

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = ER_SUCCESS;

    return (thread_errno);
}
