#include "task_scheduler_tracing.hpp"
#include "pthread.hpp"
#include "../Template/vector.hpp"
#include <atomic>
#include <mutex>

static ft_vector<task_scheduler_trace_sink> g_task_scheduler_trace_sinks(4);
static std::mutex g_task_scheduler_trace_mutex;
static std::atomic<unsigned long long> g_task_scheduler_trace_counter(1);
thread_local unsigned long long g_task_scheduler_trace_current = 0;

const char *const g_ft_task_trace_label_async = "async_task";
const char *const g_ft_task_trace_label_schedule_once = "scheduled_once";
const char *const g_ft_task_trace_label_schedule_repeat = "scheduled_repeat";

static void task_scheduler_trace_set_error(int error)
{
    ft_errno = error;
    return ;
}

int task_scheduler_register_trace_sink(task_scheduler_trace_sink sink)
{
    if (sink == ft_nullptr)
    {
        task_scheduler_trace_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    {
        std::lock_guard<std::mutex> guard(g_task_scheduler_trace_mutex);
        size_t index;
        size_t count;

        index = 0;
        count = g_task_scheduler_trace_sinks.size();
        while (index < count)
        {
            if (g_task_scheduler_trace_sinks[index] == sink)
            {
                task_scheduler_trace_set_error(FT_ERR_ALREADY_EXISTS);
                return (-1);
            }
            index += 1;
        }
        g_task_scheduler_trace_sinks.push_back(sink);
        if (g_task_scheduler_trace_sinks.get_error() != FT_ER_SUCCESSS)
        {
            task_scheduler_trace_set_error(g_task_scheduler_trace_sinks.get_error());
            return (-1);
        }
    }
    task_scheduler_trace_set_error(FT_ER_SUCCESSS);
    return (0);
}

int task_scheduler_unregister_trace_sink(task_scheduler_trace_sink sink)
{
    if (sink == ft_nullptr)
    {
        task_scheduler_trace_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    {
        std::lock_guard<std::mutex> guard(g_task_scheduler_trace_mutex);
        size_t index;
        size_t count;

        index = 0;
        count = g_task_scheduler_trace_sinks.size();
        while (index < count)
        {
            if (g_task_scheduler_trace_sinks[index] == sink)
            {
                g_task_scheduler_trace_sinks.erase(g_task_scheduler_trace_sinks.begin() + index);
                if (g_task_scheduler_trace_sinks.get_error() != FT_ER_SUCCESSS)
                {
                    task_scheduler_trace_set_error(g_task_scheduler_trace_sinks.get_error());
                    return (-1);
                }
                task_scheduler_trace_set_error(FT_ER_SUCCESSS);
                return (0);
            }
            index += 1;
        }
    }
    task_scheduler_trace_set_error(FT_ERR_NOT_FOUND);
    return (-1);
}

void task_scheduler_trace_emit(const ft_task_trace_event &event)
{
    ft_vector<task_scheduler_trace_sink> sinks_copy;

    {
        std::lock_guard<std::mutex> guard(g_task_scheduler_trace_mutex);
        size_t index;
        size_t count;

        index = 0;
        count = g_task_scheduler_trace_sinks.size();
        while (index < count)
        {
            task_scheduler_trace_sink sink_instance;

            sink_instance = g_task_scheduler_trace_sinks[index];
            sinks_copy.push_back(sink_instance);
            if (sinks_copy.get_error() != FT_ER_SUCCESSS)
            {
                task_scheduler_trace_set_error(sinks_copy.get_error());
                return ;
            }
            index += 1;
        }
    }
    size_t call_index;
    size_t call_count;

    call_index = 0;
    call_count = sinks_copy.size();
    while (call_index < call_count)
    {
        task_scheduler_trace_sink sink_instance;

        sink_instance = sinks_copy[call_index];
        if (sink_instance != ft_nullptr)
            sink_instance(event);
        call_index += 1;
    }
    task_scheduler_trace_set_error(FT_ER_SUCCESSS);
    return ;
}

unsigned long long task_scheduler_trace_generate_span_id(void)
{
    unsigned long long next_value;

    next_value = g_task_scheduler_trace_counter.fetch_add(1);
    if (next_value == 0)
        next_value = g_task_scheduler_trace_counter.fetch_add(1);
    return (next_value);
}

unsigned long long task_scheduler_trace_current_span(void)
{
    return (g_task_scheduler_trace_current);
}

unsigned long long task_scheduler_trace_push_span(unsigned long long span_id)
{
    unsigned long long previous_span;

    previous_span = g_task_scheduler_trace_current;
    g_task_scheduler_trace_current = span_id;
    return (previous_span);
}

void task_scheduler_trace_pop_span(unsigned long long previous_span)
{
    g_task_scheduler_trace_current = previous_span;
    return ;
}
