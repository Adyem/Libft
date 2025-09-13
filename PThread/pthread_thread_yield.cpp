#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int pt_thread_yield()
{
    return (cmp_thread_yield());
}
