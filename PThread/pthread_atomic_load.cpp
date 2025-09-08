#include <atomic>
#include "pthread.hpp"

int pt_atomic_load(const std::atomic<int>& atomic_variable)
{
    return (atomic_variable.load());
}

