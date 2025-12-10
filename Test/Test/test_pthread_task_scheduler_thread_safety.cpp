#include "../../PThread/task_scheduler.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_scheduled_task_state_thread_safety_resets_errno,
        "ft_scheduled_task_state enables and disables thread safety guard while resetting errno")
{
    ft_scheduled_task_state state;
    bool                   lock_acquired;

    FT_ASSERT_EQ(false, state.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, state.enable_thread_safety());
    FT_ASSERT_EQ(true, state.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, state.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    state.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    state.disable_thread_safety();
    FT_ASSERT_EQ(false, state.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_ft_scheduled_task_handle_thread_safety_resets_errno,
        "ft_scheduled_task_handle guards state access and resets errno to success")
{
    ft_scheduled_task_handle handle;
    bool                    lock_acquired;

    FT_ASSERT_EQ(false, handle.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, handle.enable_thread_safety());
    FT_ASSERT_EQ(true, handle.is_thread_safe_enabled());
    ft_errno = FT_ERR_NO_MEMORY;
    lock_acquired = false;
    FT_ASSERT_EQ(0, handle.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    handle.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    handle.disable_thread_safety();
    FT_ASSERT_EQ(false, handle.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_ft_task_scheduler_thread_safety_resets_errno,
        "ft_task_scheduler toggles guard mutex and resets errno state to success")
{
    ft_task_scheduler scheduler(1);
    bool              lock_acquired;

    FT_ASSERT_EQ(false, scheduler.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, scheduler.enable_thread_safety());
    FT_ASSERT_EQ(true, scheduler.is_thread_safe_enabled());
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    lock_acquired = false;
    FT_ASSERT_EQ(0, scheduler.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    scheduler.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    scheduler.disable_thread_safety();
    FT_ASSERT_EQ(false, scheduler.is_thread_safe_enabled());
    return (1);
}
