#include "errno.hpp"

#include <atomic>

thread_local int ft_errno = FT_ERR_SUCCESSS;

unsigned long long ft_errno_next_operation_id(void)
{
    static std::atomic<unsigned long long> next_op_id(1);

    return (next_op_id.fetch_add(1, std::memory_order_relaxed));
}
