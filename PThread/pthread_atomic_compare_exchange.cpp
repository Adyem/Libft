#include "pthread.hpp"
#include <atomic>

bool pt_atomic_compare_exchange(std::atomic<int>& atomic_variable, int& expected_value, int desired_value)
{
    return (atomic_variable.compare_exchange_strong(expected_value, desired_value));
}

