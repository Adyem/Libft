#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int pt_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (cmp_thread_equal(thread1, thread2));
}
