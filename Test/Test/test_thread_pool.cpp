#include "../../Template/thread_pool.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>

FT_TEST(test_thread_pool_resets_error_status,
    "ft_thread_pool resets error status after recovery")
{
    ft_thread_pool pool_instance(1, 0);
    std::atomic<int> execution_count;

    execution_count.store(0);
    cma_set_alloc_limit(sizeof(ft_function<void()>));
    pool_instance.submit([&execution_count]()
    {
        execution_count.store(-1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, pool_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    cma_set_alloc_limit(0);
    pool_instance.submit([&execution_count]()
    {
        execution_count.store(1);
        return ;
    });
    FT_ASSERT_EQ(ER_SUCCESS, pool_instance.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    pool_instance.wait();
    FT_ASSERT_EQ(ER_SUCCESS, pool_instance.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    int final_count;

    final_count = execution_count.load();
    FT_ASSERT_EQ(1, final_count);
    pool_instance.destroy();
    FT_ASSERT_EQ(ER_SUCCESS, pool_instance.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_set_alloc_limit(0);
    return (1);
}
