#include "../test_internal.hpp"
#include "../../Modules/Threading/task_scheduler.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Observability/observability_task_scheduler_bridge.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Time/time.hpp"
#include <atomic>
#include <chrono>
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Template/promise.hpp"
#include "../../Modules/Template/future.hpp"
#include <unistd.h>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/pthread_internal.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include <thread>
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/queue.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_task_scheduler_submit)
{
    ft_task_scheduler scheduler_instance(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    ft_future<int> future_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        scheduler_instance.submit(future_value, []() { return (7); }));
    FT_ASSERT(future_value.valid());
    FT_ASSERT_EQ(7, future_value.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
    return (1);
}

static ft_bool wait_for_counter_threshold(const std::atomic<int> &counter,
    int threshold, int32_t wait_iterations_max)
{
    int32_t wait_iterations;

    wait_iterations = 0;
    while (wait_iterations < wait_iterations_max)
    {
        if (counter.load() >= threshold)
            return (FT_TRUE);
        usleep(1000);
        wait_iterations += 1;
    }
    return (FT_FALSE);
}

static ft_bool wait_for_flag(const std::atomic<ft_bool> &flag,
    int32_t wait_timeout_milliseconds)
{
    std::chrono::steady_clock::time_point deadline;

    deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(wait_timeout_milliseconds);
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (flag.load() == FT_TRUE)
            return (FT_TRUE);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return (FT_FALSE);
}

static void reset_task_scheduler_trace_state(void)
{
    (void)observability_task_scheduler_bridge_shutdown();
    (void)cma_set_alloc_limit(0);
    return ;
}

FT_TEST(test_task_scheduler_schedule_after)
{
    reset_task_scheduler_trace_state();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    {
        t_monotonic_time_point start_time;
        t_monotonic_time_point end_time;
        long long elapsed_milliseconds;

        start_time = time_monotonic_point_now();
        auto schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(50), []() { return (3); });
        ft_future<int> future_value;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.move(schedule_result.key));
        ft_scheduled_task_handle handle_value = schedule_result.get_value();
        FT_ASSERT(handle_value.valid());
        int result_value = future_value.get();
        end_time = time_monotonic_point_now();
        elapsed_milliseconds = time_monotonic_point_diff_ms(start_time, end_time);
        FT_ASSERT_EQ(3, result_value);
        FT_ASSERT(elapsed_milliseconds >= 50);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.destroy());
    }
    return (1);
}

FT_TEST(test_task_scheduler_schedule_every)
{
    reset_task_scheduler_trace_state();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    {
        std::atomic<int> call_counter;
        int count_value;

        call_counter.store(0);
        ft_scheduled_task_handle periodic_handle = scheduler_instance.schedule_every(std::chrono::milliseconds(10), [&call_counter]()
        {
            call_counter.fetch_add(1);
            return ;
        });
        FT_ASSERT(periodic_handle.valid());
        FT_ASSERT(wait_for_counter_threshold(call_counter, 2, 30000));
        count_value = call_counter.load();
        FT_ASSERT(count_value > 1);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.destroy());
    }
    return (1);
}

FT_TEST(test_task_scheduler_queue_failure_releases_mutex)
{
    reset_task_scheduler_trace_state();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    ft_task_scheduler scheduler_instance(1);
    {
        std::atomic<int> execution_count;
        std::atomic<ft_bool> success_task_completed;
        std::atomic<ft_bool> outer_task_started;
        std::atomic<ft_bool> allow_inner_schedule;

        execution_count.store(0);
        success_task_completed.store(FT_FALSE);
        outer_task_started.store(FT_FALSE);
        allow_inner_schedule.store(FT_FALSE);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
        auto outer_schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(0),
            [&scheduler_instance, &execution_count, &outer_task_started, &allow_inner_schedule]()
        {
            execution_count.fetch_add(1);
            outer_task_started.store(FT_TRUE);
            while (allow_inner_schedule.load() == FT_FALSE)
            {
                std::this_thread::yield();
            }
            cma_set_alloc_limit(1);
            auto failure_schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(10),
                [&execution_count]()
            {
                execution_count.fetch_add(1);
                return ;
            });
            (void)failure_schedule_result;
            cma_set_alloc_limit(0);
            return ;
        });
        ft_future<void> outer_future;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, outer_future.move(outer_schedule_result.key));
        ft_scheduled_task_handle outer_handle = outer_schedule_result.get_value();
        FT_ASSERT(outer_handle.valid());
        FT_ASSERT(wait_for_flag(outer_task_started, 30000));
        allow_inner_schedule.store(FT_TRUE);
        outer_future.get();
        cma_set_alloc_limit(0);
        ft_future<void> success_future;
        FT_ASSERT_EQ(FT_ERR_SUCCESS,
            scheduler_instance.submit(success_future,
            [&success_task_completed, &execution_count]()
        {
            execution_count.fetch_add(1);
            success_task_completed.store(FT_TRUE);
            return ;
        }));
        FT_ASSERT(success_future.valid());
        success_future.get();
        FT_ASSERT(wait_for_flag(success_task_completed, 30000));
        FT_ASSERT_EQ(FT_TRUE, success_task_completed.load());
        FT_ASSERT(execution_count.load() >= 2);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.destroy());
    }
    return (1);
}
