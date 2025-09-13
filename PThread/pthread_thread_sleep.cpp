#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int pt_thread_sleep(unsigned int milliseconds)
{
    return (cmp_thread_sleep(milliseconds));
}
