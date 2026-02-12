#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int pt_thread_equal(pthread_t thread1, pthread_t thread2)
{
    int equal_result;

    equal_result = cmp_thread_equal(thread1, thread2);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (equal_result);
}
