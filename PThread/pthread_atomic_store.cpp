#include "pthread.hpp"
#include <atomic>

void pt_atomic_store(std::atomic<int>& atomic_variable, int desired_value)
{
    atomic_variable.store(desired_value);
    return ;
}

