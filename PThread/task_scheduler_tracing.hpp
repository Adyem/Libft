#ifndef TASK_SCHEDULER_TRACING_HPP
#define TASK_SCHEDULER_TRACING_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"

enum e_ft_task_trace_phase
{
    FT_TASK_TRACE_PHASE_SUBMITTED = 0,
    FT_TASK_TRACE_PHASE_ENQUEUED = 1,
    FT_TASK_TRACE_PHASE_DEQUEUED = 2,
    FT_TASK_TRACE_PHASE_STARTED = 3,
    FT_TASK_TRACE_PHASE_FINISHED = 4,
    FT_TASK_TRACE_PHASE_CANCELLED = 5,
    FT_TASK_TRACE_PHASE_TIMER_REGISTERED = 6,
    FT_TASK_TRACE_PHASE_TIMER_TRIGGERED = 7
};

struct ft_task_trace_event
{
    e_ft_task_trace_phase phase;
    unsigned long long trace_id;
    unsigned long long parent_id;
    const char *label;
    t_monotonic_time_point timestamp;
    unsigned long long thread_id;
    long long queue_depth;
    long long scheduled_depth;
    long long worker_active_count;
    long long worker_idle_count;
    bool timer_thread;
};

typedef void (*task_scheduler_trace_sink)(const ft_task_trace_event &event);

extern const char *const g_ft_task_trace_label_async;
extern const char *const g_ft_task_trace_label_schedule_once;
extern const char *const g_ft_task_trace_label_schedule_repeat;

int task_scheduler_register_trace_sink(task_scheduler_trace_sink sink);
int task_scheduler_unregister_trace_sink(task_scheduler_trace_sink sink);
void task_scheduler_trace_emit(const ft_task_trace_event &event);
unsigned long long task_scheduler_trace_generate_span_id(void);
unsigned long long task_scheduler_trace_current_span(void);
unsigned long long task_scheduler_trace_push_span(unsigned long long span_id);
void task_scheduler_trace_pop_span(unsigned long long previous_span);

#endif
