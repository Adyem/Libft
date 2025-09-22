#include "../../PThread/task_scheduler.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Time/time.hpp"
#include "../../Template/atomic.hpp"
#include <chrono>
#include "../../Errno/errno.hpp"
#include "../../Template/promise.hpp"
#include "../../Template/future.hpp"
#include <unistd.h>

FT_TEST(test_task_scheduler_submit, "ft_task_scheduler submit")
{
    ft_task_scheduler scheduler_instance(2);
    auto future_value = scheduler_instance.submit([]() { return (7); });
    FT_ASSERT_EQ(7, future_value.get());
    return (1);
}

FT_TEST(test_task_scheduler_schedule_after, "ft_task_scheduler schedule_after")
{
    ft_task_scheduler scheduler_instance(1);
    t_monotonic_time_point start_time;
    t_monotonic_time_point end_time;
    long long elapsed_milliseconds;

    start_time = time_monotonic_point_now();
    auto future_value = scheduler_instance.schedule_after(std::chrono::milliseconds(50), []() { return (3); });
    int result_value = future_value.get();
    end_time = time_monotonic_point_now();
    elapsed_milliseconds = time_monotonic_point_diff_ms(start_time, end_time);
    FT_ASSERT_EQ(3, result_value);
    FT_ASSERT(elapsed_milliseconds >= 50);
    return (1);
}

FT_TEST(test_task_scheduler_schedule_every, "ft_task_scheduler schedule_every")
{
    ft_task_scheduler scheduler_instance(1);
    ft_atomic<int> call_counter;
    call_counter.store(0);
    scheduler_instance.schedule_every(std::chrono::milliseconds(10), [&call_counter]()
    {
        call_counter.fetch_add(1);
        return ;
    });
    usleep(50000);
    int count_value = call_counter.load();
    FT_ASSERT(count_value > 1);
    return (1);
}

FT_TEST(test_task_scheduler_queue_failure_releases_mutex,
    "ft_task_scheduler handles queue push failures")
{
    ft_task_scheduler scheduler_instance(1);
    ft_promise<int> completion_promise;
    ft_future<int> completion_future(completion_promise);
    ft_atomic<int> execution_count;

    execution_count.store(0);
    scheduler_instance.schedule_after(std::chrono::milliseconds(20),
        [&scheduler_instance, &completion_promise, &execution_count]()
    {
        execution_count.fetch_add(1);
        cma_set_alloc_limit(0);
        scheduler_instance.schedule_after(std::chrono::milliseconds(10),
            [&completion_promise, &execution_count]()
        {
            execution_count.fetch_add(1);
            completion_promise.set_value(1);
            return ;
        });
        return ;
    });
    usleep(10000);
    cma_set_alloc_limit(1);
    completion_future.wait();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ER_SUCCESS, completion_future.get_error());
    FT_ASSERT_EQ(1, completion_future.get());
    FT_ASSERT_EQ(2, execution_count.load());
    return (1);
}

