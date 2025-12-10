#include "../../PThread/thread.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_thread_enable_thread_safety_allocates_mutex,
        "ft_thread enable_thread_safety installs mutex guard")
{
    ft_thread thread;

    FT_ASSERT_EQ(false, thread.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, thread.enable_thread_safety());
    FT_ASSERT_EQ(true, thread.is_thread_safe_enabled());
    thread.disable_thread_safety();
    FT_ASSERT_EQ(false, thread.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_ft_thread_lock_cycle_resets_errno,
        "ft_thread lock and unlock set ft_errno to success")
{
    ft_thread thread;
    bool      lock_acquired;

    FT_ASSERT_EQ(0, thread.enable_thread_safety());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, thread.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    thread.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    thread.disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_thread_join_respects_thread_safety,
        "ft_thread join succeeds when thread safety is enabled")
{
    ft_thread worker([]()
    {
        return ;
    });

    FT_ASSERT_EQ(FT_ER_SUCCESSS, worker.get_error());
    FT_ASSERT_EQ(0, worker.enable_thread_safety());
    FT_ASSERT_EQ(true, worker.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, worker.joinable());
    worker.join();
    FT_ASSERT_EQ(false, worker.joinable());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, worker.get_error());
    worker.disable_thread_safety();
    return (1);
}

