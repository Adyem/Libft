#include "../test_internal.hpp"
#include "../../Modules/Threading/task_scheduler.hpp"
#include "../../Modules/Threading/task_scheduler_tracing.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/pthread_internal.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/promise.hpp"
#include "../../Modules/Template/queue.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static std::mutex g_trace_mutex;
static std::vector<ft_task_trace_event> g_recorded_events;

static void task_scheduler_trace_test_sink(const ft_task_trace_event &event)
{
    std::lock_guard<std::mutex> guard(g_trace_mutex);
    g_recorded_events.push_back(event);
    return ;
}

static void task_scheduler_trace_clear_events(void)
{
    std::lock_guard<std::mutex> guard(g_trace_mutex);
    std::vector<ft_task_trace_event>().swap(g_recorded_events);
    return ;
}

struct task_scheduler_trace_test_guard
{
    ft_bool _cleaned;

    task_scheduler_trace_test_guard() : _cleaned(FT_FALSE)
    {
        return ;
    }

    int32_t cleanup(void)
    {
        int32_t unregister_result;

        if (this->_cleaned == FT_TRUE)
            return (FT_ERR_SUCCESS);
        unregister_result = task_scheduler_unregister_trace_sink(
            &task_scheduler_trace_test_sink);
        task_scheduler_trace_clear_events();
        this->_cleaned = FT_TRUE;
        return (unregister_result);
    }

    ~task_scheduler_trace_test_guard()
    {
        if (this->_cleaned == FT_FALSE)
            (void)this->cleanup();
        return ;
    }
};

static std::vector<ft_task_trace_event> task_scheduler_trace_snapshot(void)
{
    std::vector<ft_task_trace_event> snapshot;
    size_t previous_size;
    size_t stable_iterations;

    previous_size = 0;
    stable_iterations = 0;
    while (stable_iterations < 3)
    {
        {
            std::lock_guard<std::mutex> guard(g_trace_mutex);
            snapshot = g_recorded_events;
        }
        if (snapshot.size() == previous_size)
            stable_iterations += 1;
        else
        {
            stable_iterations = 0;
            previous_size = snapshot.size();
        }
        if (stable_iterations < 3)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return (snapshot);
}

FT_TEST(test_task_scheduler_tracing_submit)
{
    task_scheduler_trace_clear_events();
    FT_ASSERT_EQ(0, task_scheduler_register_trace_sink(&task_scheduler_trace_test_sink));
    task_scheduler_trace_test_guard cleanup_guard;
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    unsigned long long root_span;
    unsigned long long previous_span;

    root_span = task_scheduler_trace_generate_span_id();
    previous_span = task_scheduler_trace_push_span(root_span);
    ft_future<int> future_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        scheduler_instance.submit(future_value, []() { return (11); }));
    FT_ASSERT_EQ(11, future_value.get());
    task_scheduler_trace_pop_span(previous_span);
    std::vector<ft_task_trace_event> events = task_scheduler_trace_snapshot();
    unsigned long long traced_identifier;
    size_t index;
    size_t count;

    traced_identifier = 0;
    index = 0;
    count = events.size();
    while (index < count)
    {
        if (events[index].label == g_ft_task_trace_label_async)
        {
            traced_identifier = events[index].trace_id;
            break;
        }
        index += 1;
    }
    FT_ASSERT(traced_identifier != 0);
    std::vector<e_ft_task_trace_phase> phases;
    index = 0;
    while (index < count)
    {
        if (events[index].trace_id == traced_identifier)
        {
            phases.push_back(events[index].phase);
            FT_ASSERT_EQ(root_span, events[index].parent_id);
            FT_ASSERT(!events[index].timer_thread);
        }
        index += 1;
    }
    FT_ASSERT_EQ(static_cast<size_t>(5), phases.size());
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_SUBMITTED, phases[0]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_ENQUEUED, phases[1]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_DEQUEUED, phases[2]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_STARTED, phases[3]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_FINISHED, phases[4]);
    FT_ASSERT_EQ(0, cleanup_guard.cleanup());
    return (1);
}

FT_TEST(test_task_scheduler_tracing_schedule_after)
{
    task_scheduler_trace_clear_events();
    FT_ASSERT_EQ(0, task_scheduler_register_trace_sink(&task_scheduler_trace_test_sink));
    task_scheduler_trace_test_guard cleanup_guard;
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    unsigned long long root_span;
    unsigned long long previous_span;

    root_span = task_scheduler_trace_generate_span_id();
    previous_span = task_scheduler_trace_push_span(root_span);
    auto schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(20), []() { return (7); });
    ft_future<int> future_value;
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, future_value.initialize(ft_move(schedule_result.key)));
    FT_ASSERT_EQ(7, future_value.get());
    task_scheduler_trace_pop_span(previous_span);
    std::vector<ft_task_trace_event> events = task_scheduler_trace_snapshot();
    unsigned long long traced_identifier;
    size_t index;
    size_t count;

    traced_identifier = 0;
    index = 0;
    count = events.size();
    while (index < count)
    {
        if (events[index].label == g_ft_task_trace_label_schedule_once)
        {
            traced_identifier = events[index].trace_id;
            break;
        }
        index += 1;
    }
    FT_ASSERT(traced_identifier != 0);
    std::vector<e_ft_task_trace_phase> phases;
    std::vector<bool> timer_flags;
    index = 0;
    while (index < count)
    {
        if (events[index].trace_id == traced_identifier)
        {
            phases.push_back(events[index].phase);
            timer_flags.push_back(events[index].timer_thread);
            if (events[index].phase == FT_TASK_TRACE_PHASE_TIMER_REGISTERED)
                FT_ASSERT(!events[index].timer_thread);
            if (events[index].phase == FT_TASK_TRACE_PHASE_TIMER_TRIGGERED)
                FT_ASSERT(events[index].timer_thread);
            FT_ASSERT_EQ(root_span, events[index].parent_id);
        }
        index += 1;
    }
    FT_ASSERT_EQ(static_cast<size_t>(6), phases.size());
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_TIMER_REGISTERED, phases[0]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_TIMER_TRIGGERED, phases[1]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_ENQUEUED, phases[2]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_DEQUEUED, phases[3]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_STARTED, phases[4]);
    FT_ASSERT_EQ(FT_TASK_TRACE_PHASE_FINISHED, phases[5]);
    index = 0;
    while (index < timer_flags.size())
    {
        if (phases[index] == FT_TASK_TRACE_PHASE_ENQUEUED)
            FT_ASSERT(timer_flags[index]);
        if (phases[index] == FT_TASK_TRACE_PHASE_DEQUEUED)
            FT_ASSERT(!timer_flags[index]);
        if (phases[index] == FT_TASK_TRACE_PHASE_STARTED)
            FT_ASSERT(!timer_flags[index]);
        if (phases[index] == FT_TASK_TRACE_PHASE_FINISHED)
            FT_ASSERT(!timer_flags[index]);
        index += 1;
    }
    FT_ASSERT_EQ(0, cleanup_guard.cleanup());
    return (1);
}
