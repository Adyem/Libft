#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include <atomic>

void pt_atomic_store(std::atomic<int>& atomic_variable, int desired_value)
{
    atomic_variable.store(desired_value);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
