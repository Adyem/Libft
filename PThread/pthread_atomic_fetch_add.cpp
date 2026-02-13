#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include <atomic>

int pt_atomic_fetch_add(std::atomic<int>& atomic_variable, int increment_value)
{
    int result;

    result = atomic_variable.fetch_add(increment_value);
    return (result);
}
