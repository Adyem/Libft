#include "observability_task_scheduler_bridge.hpp"
#include "../Errno/errno.hpp"
#include "../Template/unordered_map.hpp"
#include "../PThread/mutex.hpp"

struct ft_otel_span_state
{
    bool submitted;
    bool started;
    bool timer_thread;
    const char *label;
    unsigned long long parent_id;
    t_monotonic_time_point submit_timestamp;
    t_monotonic_time_point start_timestamp;
    unsigned long long thread_id;
};

static pt_mutex g_observability_bridge_mutex;
static bool g_observability_bridge_initialized = false;
static ft_otel_span_exporter g_observability_bridge_exporter = ft_nullptr;
static ft_unordered_map<unsigned long long, ft_otel_span_state> g_observability_span_states;
static ft_otel_span_state observability_span_state_create(void)
{
    ft_otel_span_state state;

    state.submitted = false;
    state.started = false;
    state.timer_thread = false;
    state.label = ft_nullptr;
    state.parent_id = 0;
    state.submit_timestamp = time_monotonic_point_create(0);
    state.start_timestamp = time_monotonic_point_create(0);
    state.thread_id = 0;
    return (state);
}

static ft_otel_span_metrics observability_span_metrics_create(void)
{
    ft_otel_span_metrics metrics;

    metrics.span_id = 0;
    metrics.parent_span_id = 0;
    metrics.label = ft_nullptr;
    metrics.submit_timestamp = time_monotonic_point_create(0);
    metrics.start_timestamp = time_monotonic_point_create(0);
    metrics.finish_timestamp = time_monotonic_point_create(0);
    metrics.queue_duration_ms = 0;
    metrics.execution_duration_ms = 0;
    metrics.total_duration_ms = 0;
    metrics.thread_id = 0;
    metrics.queue_depth = 0;
    metrics.scheduled_depth = 0;
    metrics.worker_active_count = 0;
    metrics.worker_idle_count = 0;
    metrics.cancelled = false;
    metrics.timer_thread = false;
    return (metrics);
}

static void observability_task_scheduler_bridge_trace_sink(const ft_task_trace_event &event)
{
    ft_otel_span_metrics completed_metrics;
    ft_otel_span_exporter exporter_copy;
    bool should_emit;
    int lock_result;
    int unlock_result;

    completed_metrics = observability_span_metrics_create();
    exporter_copy = ft_nullptr;
    should_emit = false;
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (!g_observability_bridge_initialized || g_observability_bridge_exporter == ft_nullptr)
    {
        (void)g_observability_bridge_mutex.unlock();
        return ;
    }
        ft_otel_span_state new_state;
        ft_otel_span_state *state_pointer;
        ft_unordered_map<unsigned long long, ft_otel_span_state>::iterator iterator(g_observability_span_states.find(event.trace_id));
        if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
        {
            (void)g_observability_bridge_mutex.unlock();
            return ;
        }
        if (iterator == g_observability_span_states.end())
        {
            new_state = observability_span_state_create();
            g_observability_span_states.insert(event.trace_id, new_state);
            if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
            {
                (void)g_observability_bridge_mutex.unlock();
                return ;
            }
            ft_unordered_map<unsigned long long, ft_otel_span_state>::iterator
                inserted_iterator(g_observability_span_states.find(event.trace_id));
            if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS
                || inserted_iterator == g_observability_span_states.end())
            {
                (void)g_observability_bridge_mutex.unlock();
                return ;
            }
            state_pointer = &inserted_iterator->second;
        }
        else
            state_pointer = &iterator->second;
        ft_otel_span_state &state = *state_pointer;

        state.label = event.label;
        state.parent_id = event.parent_id;
        if (event.timer_thread)
            state.timer_thread = true;
        if (event.phase == FT_TASK_TRACE_PHASE_SUBMITTED)
        {
            state.submitted = true;
            state.submit_timestamp = event.timestamp;
        }
        if (event.phase == FT_TASK_TRACE_PHASE_STARTED)
        {
            state.started = true;
            state.start_timestamp = event.timestamp;
            state.thread_id = event.thread_id;
        }
        if (event.phase == FT_TASK_TRACE_PHASE_DEQUEUED)
            state.thread_id = event.thread_id;
        if (event.phase == FT_TASK_TRACE_PHASE_FINISHED || event.phase == FT_TASK_TRACE_PHASE_CANCELLED)
        {
            state.thread_id = event.thread_id;
            completed_metrics.span_id = event.trace_id;
            completed_metrics.parent_span_id = state.parent_id;
            completed_metrics.label = state.label;
            completed_metrics.submit_timestamp = state.submit_timestamp;
            completed_metrics.start_timestamp = state.start_timestamp;
            completed_metrics.finish_timestamp = event.timestamp;
            if (state.submitted && state.started)
                completed_metrics.queue_duration_ms = time_monotonic_point_diff_ms(state.submit_timestamp, state.start_timestamp);
            else
                completed_metrics.queue_duration_ms = 0;
            if (state.started)
                completed_metrics.execution_duration_ms = time_monotonic_point_diff_ms(state.start_timestamp, event.timestamp);
            else
                completed_metrics.execution_duration_ms = 0;
            if (state.submitted)
                completed_metrics.total_duration_ms = time_monotonic_point_diff_ms(state.submit_timestamp, event.timestamp);
            else
                completed_metrics.total_duration_ms = 0;
            completed_metrics.thread_id = state.thread_id;
            completed_metrics.queue_depth = event.queue_depth;
            completed_metrics.scheduled_depth = event.scheduled_depth;
            completed_metrics.worker_active_count = event.worker_active_count;
            completed_metrics.worker_idle_count = event.worker_idle_count;
            completed_metrics.cancelled = false;
            if (event.phase == FT_TASK_TRACE_PHASE_CANCELLED)
                completed_metrics.cancelled = true;
            completed_metrics.timer_thread = state.timer_thread;
            if (event.timer_thread)
                completed_metrics.timer_thread = true;
            g_observability_span_states.erase(event.trace_id);
            if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
            {
                (void)g_observability_bridge_mutex.unlock();
                return ;
            }
            exporter_copy = g_observability_bridge_exporter;
            should_emit = true;
        }
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return ;
    if (should_emit && exporter_copy != ft_nullptr)
        exporter_copy(completed_metrics);
    return ;
}

int observability_task_scheduler_bridge_initialize(ft_otel_span_exporter exporter)
{
    int lock_result;
    int unlock_result;

    if (exporter == ft_nullptr)
        return (-1);
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    if (g_observability_bridge_initialized)
    {
        g_observability_bridge_exporter = exporter;
        (void)g_observability_bridge_mutex.unlock();
        return (0);
    }
    g_observability_bridge_exporter = exporter;
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    int register_result;

    register_result = task_scheduler_register_trace_sink(&observability_task_scheduler_bridge_trace_sink);
    if (register_result != 0)
    {
        lock_result = g_observability_bridge_mutex.lock();
        if (lock_result == FT_ERR_SUCCESS)
        {
            g_observability_bridge_exporter = ft_nullptr;
            g_observability_bridge_initialized = false;
            (void)g_observability_bridge_mutex.unlock();
        }
        return (-1);
    }
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    g_observability_span_states.clear();
    if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
    {
        g_observability_bridge_exporter = ft_nullptr;
        g_observability_bridge_initialized = false;
        (void)g_observability_bridge_mutex.unlock();
        return (-1);
    }
    g_observability_bridge_initialized = true;
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int observability_task_scheduler_bridge_shutdown(void)
{
    int lock_result;
    int unlock_result;

    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    if (!g_observability_bridge_initialized)
    {
        g_observability_bridge_exporter = ft_nullptr;
        g_observability_span_states.clear();
        unlock_result = g_observability_bridge_mutex.unlock();
        if (unlock_result != FT_ERR_SUCCESS)
            return (-1);
        if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    int unregister_result;
    unregister_result = task_scheduler_unregister_trace_sink(&observability_task_scheduler_bridge_trace_sink);
    if (unregister_result != 0)
    {
        return (-1);
    }
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    g_observability_bridge_initialized = false;
    g_observability_bridge_exporter = ft_nullptr;
    g_observability_span_states.clear();
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    if (g_observability_span_states.last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
