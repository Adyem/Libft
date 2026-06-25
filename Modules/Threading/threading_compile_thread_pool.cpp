#include "thread_pool.hpp"
#include <cstdint>
#include "../Basic/limits.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t compile_thread_pool_usage()
{
    ft_thread_pool thread_pool_value(0, 0);

    if (thread_pool_value.initialize() != FT_ERR_SUCCESS)
        return (0);
    thread_pool_value.destroy();
    return (1);
}

static int32_t thread_pool_dummy = compile_thread_pool_usage();
