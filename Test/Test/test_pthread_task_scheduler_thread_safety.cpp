#include "../test_internal.hpp"
#include "../../Modules/Threading/task_scheduler.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_scheduled_task_state_thread_safety_resets_errno)
{
    ft_scheduled_task_state state;
    bool                   lock_acquired;

    FT_ASSERT_EQ(false, state.is_thread_safe());
    FT_ASSERT_EQ(0, state.enable_thread_safety());
    FT_ASSERT_EQ(true, state.is_thread_safe());
    lock_acquired = false;
    FT_ASSERT_EQ(0, state.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    state.unlock(lock_acquired);
    state.disable_thread_safety();
    FT_ASSERT_EQ(false, state.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_scheduled_task_handle_thread_safety_resets_errno)
{
    ft_scheduled_task_handle handle;
    bool                    lock_acquired;

    FT_ASSERT_EQ(false, handle.is_thread_safe());
    FT_ASSERT_EQ(0, handle.enable_thread_safety());
    FT_ASSERT_EQ(true, handle.is_thread_safe());
    lock_acquired = false;
    FT_ASSERT_EQ(0, handle.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    handle.unlock(lock_acquired);
    handle.disable_thread_safety();
    FT_ASSERT_EQ(false, handle.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_task_scheduler_thread_safety_resets_errno)
{
    ft_task_scheduler scheduler(1);
    bool              lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(false, scheduler.is_thread_safe());
    FT_ASSERT_EQ(0, scheduler.enable_thread_safety());
    FT_ASSERT_EQ(true, scheduler.is_thread_safe());
    lock_acquired = false;
    FT_ASSERT_EQ(0, scheduler.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    scheduler.unlock(lock_acquired);
    scheduler.disable_thread_safety();
    FT_ASSERT_EQ(false, scheduler.is_thread_safe());
    return (1);
}
