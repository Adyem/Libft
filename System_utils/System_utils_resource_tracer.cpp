#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Logger/logger_internal.hpp"
#include "system_utils.hpp"
#include <mutex>
#include <new>
#include <vector>

typedef std::vector<t_su_resource_tracer> t_tracer_vector;

static t_tracer_vector    g_su_resource_tracers;
static std::mutex                         g_su_resource_tracers_mutex;
static const char                        *g_su_abort_reason = ft_nullptr;
static std::mutex                         g_su_abort_reason_mutex;

static void su_log_resource_tracing_event(const char *reason)
{
    if (g_logger == ft_nullptr)
        return ;
    if (reason == ft_nullptr)
        g_logger->error("Resource tracers invoked without a reason context");
    else
        g_logger->error("Resource tracers invoked due to: %s", reason);
    return ;
}

int su_register_resource_tracer(t_su_resource_tracer tracer)
{
    bool lock_acquired;

    if (tracer == ft_nullptr)
        return (-1);
    lock_acquired = false;
    g_su_resource_tracers_mutex.lock();
    lock_acquired = true;
    size_t index = 0;
    size_t count = g_su_resource_tracers.size();
    while (index < count)
    {
        if (g_su_resource_tracers[index] == tracer)
        {
            if (lock_acquired)
                g_su_resource_tracers_mutex.unlock();
            return (-1);
        }
        index += 1;
    }
    try
    {
        g_su_resource_tracers.push_back(tracer);
    }
    catch (const std::bad_alloc &)
    {
        if (lock_acquired)
            g_su_resource_tracers_mutex.unlock();
        return (-1);
    }
    if (lock_acquired)
        g_su_resource_tracers_mutex.unlock();
    return (0);
}

int su_unregister_resource_tracer(t_su_resource_tracer tracer)
{
    bool lock_acquired;

    if (tracer == ft_nullptr)
        return (-1);
    lock_acquired = false;
    g_su_resource_tracers_mutex.lock();
    lock_acquired = true;
    size_t index = 0;
    size_t count = g_su_resource_tracers.size();
    while (index < count)
    {
        if (g_su_resource_tracers[index] == tracer)
        {
            g_su_resource_tracers.erase(g_su_resource_tracers.begin() + static_cast<t_tracer_vector::difference_type>(index));
            if (lock_acquired)
                g_su_resource_tracers_mutex.unlock();
            return (0);
        }
        index += 1;
    }
    if (lock_acquired)
        g_su_resource_tracers_mutex.unlock();
    return (-1);
}

void su_clear_resource_tracers(void)
{
    g_su_resource_tracers_mutex.lock();
    g_su_resource_tracers.clear();
    g_su_resource_tracers_mutex.unlock();
    return ;
}

void su_run_resource_tracers(const char *reason)
{
    t_tracer_vector local_tracers;
    bool lock_acquired;
    size_t count;
    size_t index;

    lock_acquired = false;
    g_su_resource_tracers_mutex.lock();
    lock_acquired = true;
    count = g_su_resource_tracers.size();
    try
    {
        local_tracers.reserve(count);
    }
    catch (const std::bad_alloc &)
    {
        if (lock_acquired)
            g_su_resource_tracers_mutex.unlock();
        return ;
    }
    index = 0;
    while (index < count)
    {
        try
        {
            local_tracers.push_back(g_su_resource_tracers[index]);
        }
        catch (const std::bad_alloc &)
        {
            if (lock_acquired)
                g_su_resource_tracers_mutex.unlock();
            return ;
        }
        index += 1;
    }
    if (lock_acquired)
        g_su_resource_tracers_mutex.unlock();
    su_log_resource_tracing_event(reason);
    size_t run_index = 0;
    size_t total = local_tracers.size();
    while (run_index < total)
    {
        t_su_resource_tracer tracer = local_tracers[run_index];
        if (tracer != ft_nullptr)
            tracer(reason);
        run_index += 1;
    }
    return ;
}

void su_internal_set_abort_reason(const char *reason)
{
    g_su_abort_reason_mutex.lock();
    g_su_abort_reason = reason;
    g_su_abort_reason_mutex.unlock();
    return ;
}

const char *su_internal_take_abort_reason(void)
{
    const char *reason;

    g_su_abort_reason_mutex.lock();
    reason = g_su_abort_reason;
    g_su_abort_reason = ft_nullptr;
    g_su_abort_reason_mutex.unlock();
    return (reason);
}
