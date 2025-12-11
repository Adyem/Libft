#include "observability_task_scheduler_bridge.hpp"
#include "../Errno/errno.hpp"
#include "../Template/unordered_map.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/lock_guard.hpp"

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
static int g_observability_bridge_error = FT_ERR_SUCCESSS;

static void observability_bridge_set_error(int error)
{
    g_observability_bridge_error = error;
    ft_errno = error;
    return ;
}

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

    completed_metrics = observability_span_metrics_create();
    exporter_copy = ft_nullptr;
    should_emit = false;
    {
        int entry_errno;
        ft_unique_lock<pt_mutex> guard(g_observability_bridge_mutex);

        entry_errno = ft_errno;
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
            return ;
        }
        if (!g_observability_bridge_initialized || g_observability_bridge_exporter == ft_nullptr)
        {
            observability_bridge_set_error(FT_ERR_SUCCESSS);
            ft_errno = entry_errno;
            return ;
        }
        ft_otel_span_state new_state;
        ft_unordered_map<unsigned long long, ft_otel_span_state>::iterator iterator(g_observability_span_states.find(event.trace_id));
        if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(g_observability_span_states.get_error());
            ft_errno = entry_errno;
            return ;
        }
        if (iterator == g_observability_span_states.end())
        {
            new_state = observability_span_state_create();
            g_observability_span_states.insert(event.trace_id, new_state);
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
            {
                observability_bridge_set_error(g_observability_span_states.get_error());
                ft_errno = entry_errno;
                return ;
            }
            iterator = g_observability_span_states.find(event.trace_id);
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS || iterator == g_observability_span_states.end())
            {
                observability_bridge_set_error(FT_ERR_INTERNAL);
                ft_errno = entry_errno;
                return ;
            }
        }
        ft_otel_span_state &state = iterator->second;

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
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
            {
                observability_bridge_set_error(g_observability_span_states.get_error());
                ft_errno = entry_errno;
                return ;
            }
            exporter_copy = g_observability_bridge_exporter;
            should_emit = true;
        }
        observability_bridge_set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
    }
    if (should_emit && exporter_copy != ft_nullptr)
        exporter_copy(completed_metrics);
    return ;
}

int observability_task_scheduler_bridge_initialize(ft_otel_span_exporter exporter)
{
    if (exporter == ft_nullptr)
    {
        observability_bridge_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    {
        int entry_errno;
        ft_lock_guard<pt_mutex> guard(g_observability_bridge_mutex);

        entry_errno = ft_errno;
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        if (g_observability_bridge_initialized)
        {
            g_observability_bridge_exporter = exporter;
            observability_bridge_set_error(FT_ERR_SUCCESSS);
            ft_errno = entry_errno;
            return (0);
        }
        g_observability_bridge_exporter = exporter;
        observability_bridge_set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
    }
    if (task_scheduler_register_trace_sink(&observability_task_scheduler_bridge_trace_sink) != 0)
    {
        observability_bridge_set_error(ft_errno);
        {
            int entry_errno;
            ft_lock_guard<pt_mutex> guard(g_observability_bridge_mutex);

            entry_errno = ft_errno;
            if (guard.get_error() == FT_ERR_SUCCESSS)
            {
                g_observability_bridge_exporter = ft_nullptr;
                g_observability_bridge_initialized = false;
            }
            else
                observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
        }
        return (-1);
    }
    {
        int entry_errno;
        ft_lock_guard<pt_mutex> guard(g_observability_bridge_mutex);

        entry_errno = ft_errno;
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        g_observability_span_states.clear();
        if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
        {
            g_observability_bridge_exporter = ft_nullptr;
            g_observability_bridge_initialized = false;
            observability_bridge_set_error(g_observability_span_states.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        g_observability_bridge_initialized = true;
        observability_bridge_set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
    }
    observability_bridge_set_error(FT_ERR_SUCCESSS);
    return (0);
}

int observability_task_scheduler_bridge_shutdown(void)
{
    {
        int entry_errno;
        ft_lock_guard<pt_mutex> guard(g_observability_bridge_mutex);

        entry_errno = ft_errno;
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        if (!g_observability_bridge_initialized)
        {
            g_observability_bridge_exporter = ft_nullptr;
            g_observability_span_states.clear();
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
            {
                observability_bridge_set_error(g_observability_span_states.get_error());
                ft_errno = entry_errno;
                return (-1);
            }
            observability_bridge_set_error(FT_ERR_SUCCESSS);
            ft_errno = entry_errno;
            return (0);
        }
        ft_errno = entry_errno;
    }
    if (task_scheduler_unregister_trace_sink(&observability_task_scheduler_bridge_trace_sink) != 0)
    {
        observability_bridge_set_error(ft_errno);
        return (-1);
    }
    {
        int entry_errno;
        ft_lock_guard<pt_mutex> guard(g_observability_bridge_mutex);

        entry_errno = ft_errno;
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(guard.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        g_observability_bridge_initialized = false;
        g_observability_bridge_exporter = ft_nullptr;
        g_observability_span_states.clear();
        if (g_observability_span_states.get_error() != FT_ERR_SUCCESSS)
        {
            observability_bridge_set_error(g_observability_span_states.get_error());
            ft_errno = entry_errno;
            return (-1);
        }
        observability_bridge_set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
    }
    observability_bridge_set_error(FT_ERR_SUCCESSS);
    return (0);
}

int observability_task_scheduler_bridge_get_error(void)
{
    return (g_observability_bridge_error);
}

const char *observability_task_scheduler_bridge_get_error_str(void)
{
    return (ft_strerror(g_observability_bridge_error));
}
