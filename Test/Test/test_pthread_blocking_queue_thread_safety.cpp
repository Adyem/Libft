#include "../../PThread/task_scheduler.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_blocking_queue_enable_thread_safety_controls,
        "ft_blocking_queue enable_thread_safety installs guard mutex")
{
    ft_blocking_queue<int> queue;

    FT_ASSERT_EQ(false, queue.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, queue.enable_thread_safety());
    FT_ASSERT_EQ(true, queue.is_thread_safe_enabled());
    queue.disable_thread_safety();
    FT_ASSERT_EQ(false, queue.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_ft_blocking_queue_lock_cycle_resets_errno,
        "ft_blocking_queue lock and unlock set ft_errno to success")
{
    ft_blocking_queue<int> queue;
    bool                   lock_acquired;

    FT_ASSERT_EQ(0, queue.enable_thread_safety());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, queue.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    queue.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    queue.disable_thread_safety();
    return (1);
}
