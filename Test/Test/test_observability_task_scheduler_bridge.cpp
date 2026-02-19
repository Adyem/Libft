#include "../test_internal.hpp"
#include "../../Observability/observability_task_scheduler_bridge.hpp"
#include "../../PThread/task_scheduler_tracing.hpp"
#include "../../System_utils/test_runner.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

#ifndef LIBFT_TEST_BUILD
#endif

static std::mutex g_span_mutex;
static std::vector<ft_otel_span_metrics> g_recorded_spans;

static void observability_test_exporter(const ft_otel_span_metrics &span)
{
    std::lock_guard<std::mutex> guard(g_span_mutex);
    g_recorded_spans.push_back(span);
    return ;
}

static void observability_test_clear_spans(void)
{
    std::lock_guard<std::mutex> guard(g_span_mutex);
    g_recorded_spans.clear();
    return ;
}

static std::vector<ft_otel_span_metrics> observability_test_snapshot(void)
{
    std::vector<ft_otel_span_metrics> spans;
    std::lock_guard<std::mutex> guard(g_span_mutex);
    size_t index;
    size_t count;

    index = 0;
    count = g_recorded_spans.size();
    while (index < count)
    {
        spans.push_back(g_recorded_spans[index]);
        index += 1;
    }
    return (spans);
}

static void observability_test_wait_for_count(size_t expected_count)
{
    size_t attempt;

    attempt = 0;
    while (attempt < 100)
    {
        std::vector<ft_otel_span_metrics> spans = observability_test_snapshot();
        if (spans.size() >= expected_count)
            return ;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        attempt += 1;
    }
    return ;
}

FT_TEST(test_observability_bridge_invalid_exporter, "observability bridge rejects null exporters")
{
    FT_ASSERT_EQ(-1, observability_task_scheduler_bridge_initialize(ft_nullptr));
    return (1);
}

FT_TEST(test_observability_bridge_async_span, "observability bridge exports async spans")
{
    ft_task_trace_event submit_event;
    ft_task_trace_event start_event;
    ft_task_trace_event finish_event;
    unsigned long long root_span;
    unsigned long long trace_id;
    t_monotonic_time_point now_point;

    observability_test_clear_spans();
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_initialize(&observability_test_exporter));
    root_span = task_scheduler_trace_generate_span_id();
    trace_id = task_scheduler_trace_generate_span_id();
    now_point = time_monotonic_point_now();

    submit_event.phase = FT_TASK_TRACE_PHASE_SUBMITTED;
    submit_event.trace_id = trace_id;
    submit_event.parent_id = root_span;
    submit_event.label = g_ft_task_trace_label_async;
    submit_event.timestamp = now_point;
    submit_event.thread_id = 0;
    submit_event.queue_depth = 1;
    submit_event.scheduled_depth = 0;
    submit_event.worker_active_count = 0;
    submit_event.worker_idle_count = 1;
    submit_event.timer_thread = false;
    task_scheduler_trace_emit(submit_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    start_event = submit_event;
    start_event.phase = FT_TASK_TRACE_PHASE_STARTED;
    start_event.timestamp = time_monotonic_point_now();
    start_event.thread_id = 42;
    start_event.queue_depth = 0;
    start_event.worker_active_count = 1;
    start_event.worker_idle_count = 0;
    task_scheduler_trace_emit(start_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    finish_event = start_event;
    finish_event.phase = FT_TASK_TRACE_PHASE_FINISHED;
    finish_event.timestamp = time_monotonic_point_now();
    task_scheduler_trace_emit(finish_event);

    observability_test_wait_for_count(1);
    std::vector<ft_otel_span_metrics> spans = observability_test_snapshot();
    FT_ASSERT_EQ(static_cast<size_t>(1), spans.size());
    const ft_otel_span_metrics &span = spans[0];
    FT_ASSERT(span.span_id != 0);
    FT_ASSERT_EQ(root_span, span.parent_span_id);
    FT_ASSERT(span.label == g_ft_task_trace_label_async);
    FT_ASSERT(span.queue_duration_ms >= 0);
    FT_ASSERT(span.execution_duration_ms >= 0);
    FT_ASSERT(span.total_duration_ms >= span.execution_duration_ms);
    FT_ASSERT(span.total_duration_ms >= span.queue_duration_ms);
    FT_ASSERT(!span.cancelled);
    FT_ASSERT(!span.timer_thread);
    FT_ASSERT(span.thread_id != 0);
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_shutdown());
    return (1);
}

FT_TEST(test_observability_bridge_cancelled_span, "observability bridge marks cancelled spans")
{
    ft_task_trace_event submit_event;
    ft_task_trace_event cancel_event;
    unsigned long long root_span;
    unsigned long long trace_id;

    observability_test_clear_spans();
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_initialize(&observability_test_exporter));
    root_span = task_scheduler_trace_generate_span_id();
    trace_id = task_scheduler_trace_generate_span_id();

    submit_event.phase = FT_TASK_TRACE_PHASE_SUBMITTED;
    submit_event.trace_id = trace_id;
    submit_event.parent_id = root_span;
    submit_event.label = g_ft_task_trace_label_schedule_once;
    submit_event.timestamp = time_monotonic_point_now();
    submit_event.thread_id = 0;
    submit_event.queue_depth = 0;
    submit_event.scheduled_depth = 1;
    submit_event.worker_active_count = 0;
    submit_event.worker_idle_count = 1;
    submit_event.timer_thread = false;
    task_scheduler_trace_emit(submit_event);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    cancel_event = submit_event;
    cancel_event.phase = FT_TASK_TRACE_PHASE_CANCELLED;
    cancel_event.timestamp = time_monotonic_point_now();
    task_scheduler_trace_emit(cancel_event);

    observability_test_wait_for_count(1);
    std::vector<ft_otel_span_metrics> spans = observability_test_snapshot();
    FT_ASSERT_EQ(static_cast<size_t>(1), spans.size());
    const ft_otel_span_metrics &span = spans[0];
    FT_ASSERT(span.span_id != 0);
    FT_ASSERT_EQ(root_span, span.parent_span_id);
    FT_ASSERT(span.label == g_ft_task_trace_label_schedule_once);
    FT_ASSERT(span.queue_duration_ms >= 0);
    FT_ASSERT_EQ(0, span.execution_duration_ms);
    FT_ASSERT(span.total_duration_ms >= 0);
    FT_ASSERT(span.cancelled);
    FT_ASSERT(!span.timer_thread);
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_shutdown());
    return (1);
}
