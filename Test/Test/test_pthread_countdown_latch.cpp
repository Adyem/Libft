#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/concurrency.hpp"
#include "../../Modules/Threading/cancellation.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_pthread_countdown_latch_releases_after_countdown)
{
    ft_countdown_latch latch;
    int32_t wait_result;
    ft_thread worker;

    wait_result = FT_ERR_INTERNAL;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, latch.initialize(1));
    worker = ft_thread([&latch, &wait_result]()
    {
        wait_result = latch.wait();
        return ;
    });
    pt_thread_sleep(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, latch.count_down());
    worker.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, wait_result);
    FT_ASSERT_EQ(static_cast<uint32_t>(0), latch.get_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, latch.destroy());
    return (1);
}

FT_TEST(test_pthread_countdown_latch_timeout_and_cancellation)
{
    ft_countdown_latch latch;
    ft_cancellation_source source;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, latch.initialize(1));
    FT_ASSERT_EQ(FT_ERR_TIMEOUT, latch.wait_for(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.request_cancel());
    FT_ASSERT_EQ(FT_ERR_TERMINATED, latch.wait(source.get_token()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, latch.destroy());
    return (1);
}
