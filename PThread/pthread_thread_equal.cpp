#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int pt_thread_equal(pthread_t thread1, pthread_t thread2)
{
    int equal_result;
    int error_code;

    equal_result = cmp_thread_equal(thread1, thread2);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (equal_result);
}
