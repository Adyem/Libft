#include "../test_internal.hpp"
#include "../../PThread/task_scheduler.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include "../../Errno/errno.hpp"
#include <chrono>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_task_scheduler_metrics_flow, "ft_task_scheduler tracks queue and worker metrics")
{
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    long long queue_size;
    long long scheduled_count;
    long long idle_count;
    long long active_count;
    size_t worker_total;

    queue_size = scheduler_instance.get_queue_size();
    FT_ASSERT_EQ(0, queue_size);
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    FT_ASSERT_EQ(0, scheduled_count);
    idle_count = scheduler_instance.get_worker_idle_count();
    FT_ASSERT_EQ(1, idle_count);
    active_count = scheduler_instance.get_worker_active_count();
    FT_ASSERT_EQ(0, active_count);
    worker_total = scheduler_instance.get_worker_total_count();
    FT_ASSERT_EQ(1, worker_total);

    std::atomic<bool> release_flag;
    release_flag.store(false);
    ft_future<void> blocking_future = scheduler_instance.submit([&release_flag]() mutable
    {
        while (!release_flag.load())
            usleep(1000);
        return ;
    });
    FT_ASSERT(blocking_future.valid());
    usleep(50000);
    active_count = scheduler_instance.get_worker_active_count();
    FT_ASSERT_EQ(1, active_count);
    idle_count = scheduler_instance.get_worker_idle_count();
    FT_ASSERT_EQ(0, idle_count);

    ft_future<void> queued_future = scheduler_instance.submit([]()
    {
        return ;
    });
    FT_ASSERT(queued_future.valid());
    usleep(20000);
    queue_size = scheduler_instance.get_queue_size();
    FT_ASSERT_EQ(1, queue_size);

    auto delayed_pair = scheduler_instance.schedule_after(std::chrono::milliseconds(200), []()
    {
        return ;
    });
    ft_scheduled_task_handle delayed_handle = delayed_pair.get_value();
    FT_ASSERT(delayed_handle.valid());
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    FT_ASSERT_EQ(1, scheduled_count);
    bool cancel_result;

    cancel_result = delayed_handle.cancel();
    FT_ASSERT(cancel_result);
    usleep(20000);
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    FT_ASSERT_EQ(0, scheduled_count);

    release_flag.store(true);
    blocking_future.get();
    queued_future.get();
    usleep(20000);
    queue_size = scheduler_instance.get_queue_size();
    FT_ASSERT_EQ(0, queue_size);
    active_count = scheduler_instance.get_worker_active_count();
    FT_ASSERT_EQ(0, active_count);
    idle_count = scheduler_instance.get_worker_idle_count();
    FT_ASSERT_EQ(1, idle_count);
    worker_total = scheduler_instance.get_worker_total_count();
    FT_ASSERT_EQ(1, worker_total);
    return (1);
}
