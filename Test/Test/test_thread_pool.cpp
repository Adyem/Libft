#include "../test_internal.hpp"
#include "../../Template/thread_pool.hpp"
#include "../../Template/cancellation.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

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
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_thread_pool::last_operation_error());
    cma_set_alloc_limit(0);
    pool_instance.submit([&execution_count]()
    {
        execution_count.store(1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_thread_pool::last_operation_error());
    pool_instance.wait();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_thread_pool::last_operation_error());
    int final_count;

    final_count = execution_count.load();
    FT_ASSERT_EQ(1, final_count);
    pool_instance.destroy();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_thread_pool::last_operation_error());
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_thread_pool_cancellation_skips_tasks,
    "ft_thread_pool skips execution when cancellation is requested")
{
    ft_thread_pool pool_instance(1, 0);
    ft_cancellation_source cancellation_source;
    const ft_cancellation_token &cancellation_token = cancellation_source.get_token();
    std::atomic<int> execution_count;

    execution_count.store(0);
    cancellation_source.request_cancel();
    pool_instance.submit([&execution_count, &cancellation_token]()
    {
        if (!cancellation_token.is_cancellation_requested())
            execution_count.fetch_add(1);
        else
            execution_count.fetch_sub(1);
        return ;
    }, cancellation_token);
    pool_instance.wait();
    FT_ASSERT_EQ(0, execution_count.load());
    pool_instance.destroy();
    return (1);
}

FT_TEST(test_thread_pool_cancellation_allows_execution,
    "ft_thread_pool executes tasks when cancellation is not requested")
{
    ft_thread_pool pool_instance(1, 0);
    ft_cancellation_source cancellation_source;
    const ft_cancellation_token &cancellation_token = cancellation_source.get_token();
    std::atomic<int> execution_count;

    execution_count.store(0);
    pool_instance.submit([&execution_count, &cancellation_token]()
    {
        if (!cancellation_token.is_cancellation_requested())
            execution_count.fetch_add(1);
        return ;
    }, cancellation_token);
    pool_instance.wait();
    FT_ASSERT_EQ(1, execution_count.load());
    pool_instance.destroy();
    return (1);
}

FT_TEST(test_cancellation_token_callbacks_trigger,
    "ft_cancellation_token invokes registered callbacks on cancellation")
{
    ft_cancellation_source cancellation_source;
    const ft_cancellation_token &cancellation_token = cancellation_source.get_token();
    std::atomic<int> callback_count;
    int registration_status;

    callback_count.store(0);
    registration_status = cancellation_token.register_callback([&callback_count]()
    {
        callback_count.fetch_add(1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registration_status);
    cancellation_source.request_cancel();
    FT_ASSERT_EQ(1, callback_count.load());
    registration_status = cancellation_token.register_callback([&callback_count]()
    {
        callback_count.fetch_add(1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registration_status);
    FT_ASSERT_EQ(2, callback_count.load());
    return (1);
}
