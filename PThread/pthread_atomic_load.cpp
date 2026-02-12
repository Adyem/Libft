#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include <atomic>

int pt_atomic_load(const std::atomic<int>& atomic_variable)
{
    int result;

    result = atomic_variable.load();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}
