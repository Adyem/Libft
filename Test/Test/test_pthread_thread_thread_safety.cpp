#include "../test_internal.hpp"
#include "../../Modules/PThread/thread.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_thread_enable_thread_safety_allocates_mutex)
{
    ft_thread thread;

    FT_ASSERT_EQ(false, thread.is_thread_safe());
    FT_ASSERT_EQ(0, thread.enable_thread_safety());
    FT_ASSERT_EQ(true, thread.is_thread_safe());
    thread.disable_thread_safety();
    FT_ASSERT_EQ(false, thread.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_thread_lock_cycle_resets_errno)
{
    ft_thread thread;
    bool      lock_acquired;

    FT_ASSERT_EQ(0, thread.enable_thread_safety());
    lock_acquired = false;
    FT_ASSERT_EQ(0, thread.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    thread.unlock(lock_acquired);
    thread.disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_thread_join_respects_thread_safety)
{
    ft_thread worker([]()
    {
        return ;
    });

    FT_ASSERT_EQ(0, worker.enable_thread_safety());
    FT_ASSERT_EQ(true, worker.is_thread_safe());
    FT_ASSERT_EQ(true, worker.joinable());
    worker.join();
    FT_ASSERT_EQ(false, worker.joinable());
    worker.disable_thread_safety();
    return (1);
}
