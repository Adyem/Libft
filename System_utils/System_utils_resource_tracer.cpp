#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Logger/logger_internal.hpp"
#include "../Template/vector.hpp"
#include "system_utils.hpp"
#include <mutex>
#include <vector>

static ft_vector<t_su_resource_tracer>    g_su_resource_tracers(4);
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
    if (tracer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    std::lock_guard<std::mutex> guard(g_su_resource_tracers_mutex);
    size_t index = 0;
    size_t count = g_su_resource_tracers.size();
    while (index < count)
    {
        if (g_su_resource_tracers[index] == tracer)
        {
            ft_errno = FT_ERR_ALREADY_EXISTS;
            return (-1);
        }
        index += 1;
    }
    g_su_resource_tracers.push_back(tracer);
    if (g_su_resource_tracers.get_error() != ER_SUCCESS)
    {
        ft_errno = g_su_resource_tracers.get_error();
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int su_unregister_resource_tracer(t_su_resource_tracer tracer)
{
    if (tracer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    std::lock_guard<std::mutex> guard(g_su_resource_tracers_mutex);
    size_t index = 0;
    size_t count = g_su_resource_tracers.size();
    while (index < count)
    {
        if (g_su_resource_tracers[index] == tracer)
        {
            g_su_resource_tracers.erase(g_su_resource_tracers.begin() + index);
            if (g_su_resource_tracers.get_error() != ER_SUCCESS)
            {
                ft_errno = g_su_resource_tracers.get_error();
                return (-1);
            }
            ft_errno = ER_SUCCESS;
            return (0);
        }
        index += 1;
    }
    ft_errno = FT_ERR_NOT_FOUND;
    return (-1);
}

void su_clear_resource_tracers(void)
{
    std::lock_guard<std::mutex> guard(g_su_resource_tracers_mutex);
    g_su_resource_tracers.clear();
    ft_errno = ER_SUCCESS;
    return ;
}

void su_run_resource_tracers(const char *reason)
{
    std::vector<t_su_resource_tracer> local_tracers;
    {
        std::lock_guard<std::mutex> guard(g_su_resource_tracers_mutex);
        size_t index = 0;
        size_t count = g_su_resource_tracers.size();
        while (index < count)
        {
            local_tracers.push_back(g_su_resource_tracers[index]);
            index += 1;
        }
    }
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
    ft_errno = ER_SUCCESS;
    return ;
}

void su_internal_set_abort_reason(const char *reason)
{
    std::lock_guard<std::mutex> guard(g_su_abort_reason_mutex);
    g_su_abort_reason = reason;
    return ;
}

const char *su_internal_take_abort_reason(void)
{
    const char *reason;

    std::lock_guard<std::mutex> guard(g_su_abort_reason_mutex);
    reason = g_su_abort_reason;
    g_su_abort_reason = ft_nullptr;
    return (reason);
}
