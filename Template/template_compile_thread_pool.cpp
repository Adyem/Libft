#include "thread_pool.hpp"

static int compile_thread_pool_usage()
{
    ft_thread_pool thread_pool_value(0, 0);

    if (thread_pool_value.initialize() != FT_ERR_SUCCESS)
        return (0);
    thread_pool_value.destroy();
    return (1);
}

static int thread_pool_dummy = compile_thread_pool_usage();
