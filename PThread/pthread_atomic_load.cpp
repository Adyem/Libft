#include "pthread.hpp"
#include <atomic>

int pt_atomic_load(const std::atomic<int>& atomic_variable)
{
    return (atomic_variable.load());
}

