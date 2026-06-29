#include "../test_internal.hpp"
#include "../../Modules/Threading/task_scheduler.hpp"
#include "../../Modules/Threading/task_scheduler_tracing.hpp"
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
#include <cstdio>

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

static void print_scheduler_counter_diagnostics(const char *label,
    const ft_task_scheduler &scheduler_instance, int call_counter)
{
    (void)label;
    (void)scheduler_instance;
    (void)call_counter;
    return ;
}

static ft_bool wait_for_counter_threshold_with_diagnostics(const char *label,
    const std::atomic<int> &counter, int threshold,
    const ft_task_scheduler &scheduler_instance,
    int32_t wait_timeout_milliseconds)
{
    std::chrono::steady_clock::time_point deadline;
    std::chrono::steady_clock::time_point last_report;

    deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(wait_timeout_milliseconds);
    last_report = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (counter.load() >= threshold)
            return (FT_TRUE);
        if (std::chrono::steady_clock::now() - last_report
            >= std::chrono::milliseconds(5000))
        {
            print_scheduler_counter_diagnostics(label, scheduler_instance,
                counter.load());
            last_report = std::chrono::steady_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    print_scheduler_counter_diagnostics(label, scheduler_instance,
        counter.load());
    return (FT_FALSE);
}

static void print_scheduler_diagnostics(const char *label,
    const ft_task_scheduler &scheduler_instance, ft_bool task_completed,
    ft_bool outer_task_started, ft_bool allow_inner_schedule,
    ft_bool inner_schedule_attempted, ft_bool inner_schedule_failed)
{
    (void)label;
    (void)scheduler_instance;
    (void)task_completed;
    (void)outer_task_started;
    (void)allow_inner_schedule;
    (void)inner_schedule_attempted;
    (void)inner_schedule_failed;
    return ;
}

static ft_bool wait_for_flag_with_diagnostics(const char *label,
    const std::atomic<ft_bool> &flag, const ft_task_scheduler &scheduler_instance,
    int32_t wait_timeout_milliseconds, ft_bool outer_task_started,
    ft_bool allow_inner_schedule, ft_bool inner_schedule_attempted,
    ft_bool inner_schedule_failed)
{
    std::chrono::steady_clock::time_point deadline;
    std::chrono::steady_clock::time_point last_report;

    deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(wait_timeout_milliseconds);
    last_report = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (flag.load() == FT_TRUE)
            return (FT_TRUE);
        if (std::chrono::steady_clock::now() - last_report
            >= std::chrono::milliseconds(5000))
        {
            print_scheduler_diagnostics(label, scheduler_instance,
                flag.load(), outer_task_started, allow_inner_schedule,
                inner_schedule_attempted, inner_schedule_failed);
            last_report = std::chrono::steady_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    print_scheduler_diagnostics(label, scheduler_instance, flag.load(),
        outer_task_started, allow_inner_schedule, inner_schedule_attempted,
        inner_schedule_failed);
    return (FT_FALSE);
}

static void reset_task_scheduler_trace_state(void)
{
    (void)observability_task_scheduler_bridge_shutdown();
    (void)task_scheduler_trace_reset_for_tests();
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
        ft_future<int> bootstrap_future;
        t_monotonic_time_point start_time;
        t_monotonic_time_point end_time;
        long long elapsed_milliseconds;
        std::atomic<ft_bool> task_completed;

        FT_ASSERT_EQ(FT_ERR_SUCCESS,
            scheduler_instance.submit(bootstrap_future, []()
        {
            return (1);
        }));
        FT_ASSERT_EQ(1, bootstrap_future.get());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, bootstrap_future.destroy());
        task_completed.store(FT_FALSE);
        start_time = time_monotonic_point_now();
        auto schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(20),
            [&task_completed]()
        {
            task_completed.store(FT_TRUE);
            return (3);
        });
        ft_future<int> future_value;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.initialize(ft_move(schedule_result.key)));
        FT_ASSERT(future_value.valid());
        ft_scheduled_task_handle handle_value = schedule_result.get_value();
        FT_ASSERT(handle_value.valid());
        if (wait_for_flag_with_diagnostics("schedule_after",
                task_completed, scheduler_instance, 300000, FT_FALSE,
                FT_FALSE, FT_FALSE, FT_FALSE) != FT_TRUE)
        {
            (void)future_value.destroy();
            (void)scheduler_instance.destroy();
            FT_ASSERT(FT_FALSE);
        }
        int result_value = future_value.get();
        end_time = time_monotonic_point_now();
        elapsed_milliseconds = time_monotonic_point_diff_ms(start_time, end_time);
        FT_ASSERT_EQ(3, result_value);
        FT_ASSERT(elapsed_milliseconds >= 20);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.destroy());
        handle_value = ft_scheduled_task_handle();
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
        if (wait_for_counter_threshold_with_diagnostics("schedule_every",
                call_counter, 2, scheduler_instance, 30000) != FT_TRUE)
        {
            (void)periodic_handle.cancel();
            (void)scheduler_instance.destroy();
            FT_ASSERT(FT_FALSE);
        }
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
        std::atomic<ft_bool> inner_schedule_attempted;
        std::atomic<ft_bool> inner_schedule_failed;

        execution_count.store(0);
        success_task_completed.store(FT_FALSE);
        outer_task_started.store(FT_FALSE);
        allow_inner_schedule.store(FT_FALSE);
        inner_schedule_attempted.store(FT_FALSE);
        inner_schedule_failed.store(FT_FALSE);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
        ft_future<void> outer_future;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.submit(outer_future,
            [&scheduler_instance, &execution_count, &outer_task_started,
                &allow_inner_schedule, &inner_schedule_attempted,
                &inner_schedule_failed]()
        {
            execution_count.fetch_add(1);
            outer_task_started.store(FT_TRUE);
            while (allow_inner_schedule.load() == FT_FALSE)
            {
                std::this_thread::yield();
            }
            inner_schedule_attempted.store(FT_TRUE);
            cma_set_alloc_limit(1);
            auto failure_schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(10),
                [&execution_count]()
            {
                execution_count.fetch_add(1);
                return ;
            });
            if (failure_schedule_result.key.valid() == FT_FALSE)
                inner_schedule_failed.store(FT_TRUE);
            (void)failure_schedule_result;
            cma_set_alloc_limit(0);
            return ;
        }));
        if (wait_for_flag_with_diagnostics("queue_failure_outer_task",
                outer_task_started, scheduler_instance, 300000,
                outer_task_started.load(), allow_inner_schedule.load(),
                inner_schedule_attempted.load(), inner_schedule_failed.load())
            != FT_TRUE)
        {
            allow_inner_schedule.store(FT_TRUE);
            cma_set_alloc_limit(0);
            (void)outer_future.destroy();
            (void)scheduler_instance.destroy();
            FT_ASSERT(FT_FALSE);
        }
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
        FT_ASSERT_EQ(FT_TRUE, success_task_completed.load());
        FT_ASSERT(execution_count.load() >= 2);
        print_scheduler_diagnostics("queue_failure_complete", scheduler_instance,
            success_task_completed.load(), outer_task_started.load(),
            allow_inner_schedule.load(), inner_schedule_attempted.load(),
            inner_schedule_failed.load());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.destroy());
    }
    return (1);
}
