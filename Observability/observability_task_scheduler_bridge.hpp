#ifndef OBSERVABILITY_TASK_SCHEDULER_BRIDGE_HPP
#define OBSERVABILITY_TASK_SCHEDULER_BRIDGE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/task_scheduler_tracing.hpp"
#include "../Time/time.hpp"

struct ft_otel_span_metrics
{
    unsigned long long span_id;
    unsigned long long parent_span_id;
    const char *label;
    t_monotonic_time_point submit_timestamp;
    t_monotonic_time_point start_timestamp;
    t_monotonic_time_point finish_timestamp;
    long long queue_duration_ms;
    long long execution_duration_ms;
    long long total_duration_ms;
    unsigned long long thread_id;
    long long queue_depth;
    long long scheduled_depth;
    long long worker_active_count;
    long long worker_idle_count;
    bool cancelled;
    bool timer_thread;
};

typedef void (*ft_otel_span_exporter)(const ft_otel_span_metrics &span);

int observability_task_scheduler_bridge_initialize(ft_otel_span_exporter exporter);
int observability_task_scheduler_bridge_shutdown(void);
int observability_task_scheduler_bridge_get_error(void);
const char *observability_task_scheduler_bridge_get_error_str(void);

#endif
