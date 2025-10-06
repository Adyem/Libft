#include "pthread.hpp"
#include <atomic>

int pt_atomic_fetch_add(std::atomic<int>& atomic_variable, int increment_value)
{
    return (atomic_variable.fetch_add(increment_value));
}

