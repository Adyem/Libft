#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int pt_thread_cancel(pthread_t thread)
{
    return (cmp_thread_cancel(thread));
}
