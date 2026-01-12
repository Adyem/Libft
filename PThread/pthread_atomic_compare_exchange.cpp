#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include <atomic>

bool pt_atomic_compare_exchange(std::atomic<int>& atomic_variable, int& expected_value, int desired_value)
{
    bool result;

    result = atomic_variable.compare_exchange_strong(expected_value, desired_value);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
