#include "PThread.hpp"

int pt_thread_equal(pthread_t thread1, pthread_t thread2)
{
#ifdef _WIN32
    return (thread1 == thread2);
#else
    return (pthread_equal(thread1, thread2));
#endif
}

