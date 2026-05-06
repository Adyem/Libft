#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <errno.h>

int pt_thread_cancel(pthread_t thread)
{
    int return_value;

    return_value = cmp_thread_cancel(thread);
    if (return_value != 0)
        return (return_value);
    return (return_value);
}
