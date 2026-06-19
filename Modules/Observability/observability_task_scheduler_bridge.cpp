#include "observability_task_scheduler_bridge.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Template/unordered_map.hpp"
#include "../PThread/mutex.hpp"
#include <pthread.h>
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

struct ft_otel_span_state
{
    ft_bool submitted;
    ft_bool started;
    ft_bool timer_thread;
    const char *label;
    uint64_t parent_id;
    t_monotonic_time_point submit_timestamp;
    t_monotonic_time_point start_timestamp;
    uint64_t thread_id;
};

static pt_mutex g_observability_bridge_mutex;
static pthread_once_t g_observability_bridge_once = PTHREAD_ONCE_INIT;
static int32_t g_observability_bridge_once_error = FT_ERR_SUCCESS;
static ft_bool g_observability_bridge_initialised = FT_FALSE;
static ft_otel_span_exporter g_observability_bridge_exporter = ft_nullptr;
static ft_unordered_map<uint64_t, ft_otel_span_state> g_observability_span_states;

#ifdef LIBFT_TEST_BUILD
static void observability_task_scheduler_bridge_untrack_runtime_leaks(void)
{
    if (g_observability_bridge_mutex._native_mutex != ft_nullptr)
        (void)cma_untrack_leak(g_observability_bridge_mutex._native_mutex);
    if (g_observability_span_states.is_initialised() == FT_CLASS_STATE_INITIALISED
        && g_observability_span_states._data != ft_nullptr)
        (void)cma_untrack_leak(g_observability_span_states._data);
    if (g_observability_span_states.is_initialised() == FT_CLASS_STATE_INITIALISED
        && g_observability_span_states._occupied != ft_nullptr)
        (void)cma_untrack_leak(g_observability_span_states._occupied);
    if (g_observability_span_states.is_initialised() == FT_CLASS_STATE_INITIALISED
        && g_observability_span_states._mutex != ft_nullptr)
        (void)cma_untrack_leak(g_observability_span_states._mutex);
    return ;
}
#endif

static void observability_task_scheduler_bridge_initialize_once(void)
{
    int32_t initialize_error;

    initialize_error = g_observability_bridge_mutex.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        g_observability_bridge_once_error = initialize_error;
        return ;
    }
    initialize_error = g_observability_span_states.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        g_observability_bridge_once_error = initialize_error;
        return ;
    }
#ifdef LIBFT_TEST_BUILD
    observability_task_scheduler_bridge_untrack_runtime_leaks();
#endif
    g_observability_bridge_once_error = FT_ERR_SUCCESS;
    return ;
}

static int32_t observability_task_scheduler_bridge_ensure_runtime(void)
{
    int32_t once_status;

    once_status = pthread_once(&g_observability_bridge_once,
        observability_task_scheduler_bridge_initialize_once);
    if (once_status != 0)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    if (g_observability_bridge_once_error != FT_ERR_SUCCESS)
        return (g_observability_bridge_once_error);
    return (FT_ERR_SUCCESS);
}
static ft_otel_span_state observability_span_state_create(void)
{
    ft_otel_span_state state;

    state.submitted = FT_FALSE;
    state.started = FT_FALSE;
    state.timer_thread = FT_FALSE;
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
    metrics.cancelled = FT_FALSE;
    metrics.timer_thread = FT_FALSE;
    return (metrics);
}

static void observability_task_scheduler_bridge_trace_sink(const ft_task_trace_event &event)
{
    ft_otel_span_metrics completed_metrics;
    ft_otel_span_exporter exporter_copy;
    ft_bool should_emit;
    int32_t lock_result;
    int32_t unlock_result;
    int32_t runtime_error;

    runtime_error = observability_task_scheduler_bridge_ensure_runtime();
    if (runtime_error != FT_ERR_SUCCESS)
        return ;
    completed_metrics = observability_span_metrics_create();
    exporter_copy = ft_nullptr;
    should_emit = FT_FALSE;
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (!g_observability_bridge_initialised || g_observability_bridge_exporter == ft_nullptr)
    {
        (void)g_observability_bridge_mutex.unlock();
        return ;
    }
        ft_otel_span_state new_state;
        ft_otel_span_state *state_pointer;
        ft_unordered_map<uint64_t, ft_otel_span_state>::iterator iterator(g_observability_span_states.find(event.trace_id));
        if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
        {
            (void)g_observability_bridge_mutex.unlock();
            return ;
        }
        if (iterator == g_observability_span_states.end())
        {
            new_state = observability_span_state_create();
            g_observability_span_states.insert(event.trace_id, new_state);
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
            {
                (void)g_observability_bridge_mutex.unlock();
                return ;
            }
#ifdef LIBFT_TEST_BUILD
            observability_task_scheduler_bridge_untrack_runtime_leaks();
#endif
            ft_unordered_map<uint64_t, ft_otel_span_state>::iterator
                inserted_iterator(g_observability_span_states.find(event.trace_id));
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESS
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
            state.timer_thread = FT_TRUE;
        if (event.phase == FT_TASK_TRACE_PHASE_SUBMITTED)
        {
            state.submitted = FT_TRUE;
            state.submit_timestamp = event.timestamp;
        }
        if (event.phase == FT_TASK_TRACE_PHASE_STARTED)
        {
            state.started = FT_TRUE;
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
            completed_metrics.cancelled = FT_FALSE;
            if (event.phase == FT_TASK_TRACE_PHASE_CANCELLED)
                completed_metrics.cancelled = FT_TRUE;
            completed_metrics.timer_thread = state.timer_thread;
            if (event.timer_thread)
                completed_metrics.timer_thread = FT_TRUE;
            g_observability_span_states.erase(event.trace_id);
            if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
            {
                (void)g_observability_bridge_mutex.unlock();
                return ;
            }
            exporter_copy = g_observability_bridge_exporter;
            should_emit = FT_TRUE;
        }
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return ;
    if (should_emit && exporter_copy != ft_nullptr)
        exporter_copy(completed_metrics);
    return ;
}

int32_t observability_task_scheduler_bridge_initialize(ft_otel_span_exporter exporter)
{
    int32_t lock_result;
    int32_t unlock_result;
    int32_t runtime_error;

    if (exporter == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    runtime_error = observability_task_scheduler_bridge_ensure_runtime();
    if (runtime_error != FT_ERR_SUCCESS)
        return (runtime_error);
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    if (g_observability_bridge_initialised)
    {
        g_observability_bridge_exporter = exporter;
        (void)g_observability_bridge_mutex.unlock();
        return (FT_ERR_SUCCESS);
    }
    g_observability_bridge_exporter = exporter;
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    int32_t register_result;

    register_result = task_scheduler_register_trace_sink(&observability_task_scheduler_bridge_trace_sink);
    if (register_result != FT_ERR_SUCCESS)
    {
        lock_result = g_observability_bridge_mutex.lock();
        if (lock_result == FT_ERR_SUCCESS)
        {
            g_observability_bridge_exporter = ft_nullptr;
            g_observability_bridge_initialised = FT_FALSE;
            (void)g_observability_bridge_mutex.unlock();
        }
        return (FT_ERR_INVALID_OPERATION);
    }
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_span_states.clear();
    if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
    {
        g_observability_bridge_exporter = ft_nullptr;
        g_observability_bridge_initialised = FT_FALSE;
        (void)g_observability_bridge_mutex.unlock();
        return (FT_ERR_NO_MEMORY);
    }
    g_observability_bridge_initialised = FT_TRUE;
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    return (FT_ERR_SUCCESS);
}

int32_t observability_task_scheduler_bridge_shutdown(void)
{
    int32_t lock_result;
    int32_t unlock_result;
    int32_t runtime_error;
    int32_t unregister_result;

    runtime_error = observability_task_scheduler_bridge_ensure_runtime();
    if (runtime_error != FT_ERR_SUCCESS)
        return (runtime_error);
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    unregister_result = task_scheduler_unregister_trace_sink(&observability_task_scheduler_bridge_trace_sink);
    if (!g_observability_bridge_initialised)
    {
        g_observability_bridge_exporter = ft_nullptr;
        g_observability_span_states.clear();
        unlock_result = g_observability_bridge_mutex.unlock();
        if (unlock_result != FT_ERR_SUCCESS)
            return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        return (FT_ERR_SUCCESS);
    }
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    if (unregister_result != FT_ERR_SUCCESS)
    {
        return (FT_ERR_INVALID_OPERATION);
    }
    lock_result = g_observability_bridge_mutex.lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_bridge_initialised = FT_FALSE;
    g_observability_bridge_exporter = ft_nullptr;
    g_observability_span_states.clear();
    unlock_result = g_observability_bridge_mutex.unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    if (g_observability_span_states.get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}
