#include "../test_internal.hpp"
#include "../../Observability/observability_task_scheduler_bridge.hpp"
#include "../../PThread/task_scheduler.hpp"
#include "../../Errno/errno.hpp"
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
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, observability_task_scheduler_bridge_get_error());
    FT_ASSERT(observability_task_scheduler_bridge_get_error_str() != ft_nullptr);
    return (1);
}

FT_TEST(test_observability_bridge_async_span, "observability bridge exports async spans")
{
    observability_test_clear_spans();
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_initialize(&observability_test_exporter));
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    unsigned long long root_span;
    unsigned long long previous_span;

    root_span = task_scheduler_trace_generate_span_id();
    previous_span = task_scheduler_trace_push_span(root_span);
    auto future_value = scheduler_instance.submit([]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return (13);
    });
    FT_ASSERT_EQ(13, future_value.get());
    task_scheduler_trace_pop_span(previous_span);
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
    observability_test_clear_spans();
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_initialize(&observability_test_exporter));
    ft_task_scheduler scheduler_instance(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance.initialize());
    unsigned long long root_span;
    unsigned long long previous_span;

    root_span = task_scheduler_trace_generate_span_id();
    previous_span = task_scheduler_trace_push_span(root_span);
    auto schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(200), []() { return (9); });
    ft_future<int> future_value = schedule_result.get_key();
    ft_scheduled_task_handle handle_instance = schedule_result.get_value();
    task_scheduler_trace_pop_span(previous_span);
    FT_ASSERT(handle_instance.cancel());
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
    (void)future_value;
    FT_ASSERT_EQ(0, observability_task_scheduler_bridge_shutdown());
    return (1);
}
