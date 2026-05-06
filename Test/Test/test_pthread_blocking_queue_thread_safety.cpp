#include "../test_internal.hpp"
#include "../../Modules/PThread/task_scheduler.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_blocking_queue_enable_thread_safety_controls)
{
    ft_blocking_queue<int32_t> queue;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    FT_ASSERT_EQ(false, queue.is_thread_safe());
    FT_ASSERT_EQ(0, queue.enable_thread_safety());
    FT_ASSERT_EQ(true, queue.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.disable_thread_safety());
    FT_ASSERT_EQ(false, queue.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.destroy());
    return (1);
}

FT_TEST(test_ft_blocking_queue_lock_cycle_resets_errno)
{
    ft_blocking_queue<int32_t> queue;
    bool lock_acquired;
    int32_t value;

    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.initialize());
    FT_ASSERT_EQ(0, queue.enable_thread_safety());
    FT_ASSERT_EQ(0, queue.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    queue.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.push(42));
    FT_ASSERT_EQ(true, queue.pop(value));
    FT_ASSERT_EQ(42, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queue.destroy());
    return (1);
}
