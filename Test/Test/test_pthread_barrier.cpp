#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/concurrency.hpp"
#include "../../Modules/Threading/cancellation.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <atomic>

FT_TEST(test_pthread_barrier_releases_all_participants)
{
    ft_barrier barrier;
    std::atomic<int32_t> passed_count;
    ft_thread worker;
    int32_t main_result;

    passed_count.store(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, barrier.initialize(2));
    worker = ft_thread([&barrier, &passed_count]()
    {
        if (barrier.arrive_and_wait() == FT_ERR_SUCCESS)
            passed_count.fetch_add(1);
        return ;
    });
    main_result = barrier.arrive_and_wait();
    worker.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, main_result);
    FT_ASSERT_EQ(1, passed_count.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, barrier.destroy());
    return (1);
}

FT_TEST(test_pthread_barrier_rejects_zero_and_observes_cancel)
{
    ft_barrier barrier;
    ft_cancellation_source source;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, barrier.initialize(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, barrier.initialize(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.request_cancel());
    FT_ASSERT_EQ(FT_ERR_TERMINATED, barrier.arrive_and_wait(source.get_token()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, barrier.destroy());
    return (1);
}
