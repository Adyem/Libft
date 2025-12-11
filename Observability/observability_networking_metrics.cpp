#include "observability_networking_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/lock_guard.hpp"

static pt_mutex g_observability_networking_mutex;
static bool g_observability_networking_initialized = false;
static ft_networking_observability_exporter g_observability_networking_exporter = ft_nullptr;
static int g_observability_networking_error = FT_ERR_SUCCESSS;

static void observability_networking_metrics_set_error(int error_code)
{
    g_observability_networking_error = error_code;
    ft_errno = error_code;
    return ;
}

int observability_networking_metrics_initialize(ft_networking_observability_exporter exporter)
{
    int entry_errno;
    int result;

    if (exporter == ft_nullptr)
    {
        observability_networking_metrics_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    entry_errno = ft_errno;
    result = 0;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_networking_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_networking_metrics_set_error(guard.get_error());
            result = -1;
        }
        else
        {
            g_observability_networking_exporter = exporter;
            g_observability_networking_initialized = true;
            observability_networking_metrics_set_error(FT_ERR_SUCCESSS);
        }
    }
    ft_errno = entry_errno;
    return (result);
}

int observability_networking_metrics_shutdown(void)
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = 0;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_networking_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_networking_metrics_set_error(guard.get_error());
            result = -1;
        }
        else
        {
            g_observability_networking_initialized = false;
            g_observability_networking_exporter = ft_nullptr;
            observability_networking_metrics_set_error(FT_ERR_SUCCESSS);
        }
    }
    ft_errno = entry_errno;
    return (result);
}

int observability_networking_metrics_get_error(void)
{
    return (g_observability_networking_error);
}

const char *observability_networking_metrics_get_error_str(void)
{
    return (ft_strerror(g_observability_networking_error));
}

void observability_networking_metrics_record(const ft_networking_observability_sample &sample)
{
    ft_networking_observability_sample exported_sample;
    ft_networking_observability_exporter exporter_copy;
    bool should_emit;
    int entry_errno;
    bool guard_failed;

    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = false;
    entry_errno = ft_errno;
    guard_failed = false;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_networking_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_networking_metrics_set_error(guard.get_error());
            guard_failed = true;
        }
        else
        {
            if (g_observability_networking_initialized && g_observability_networking_exporter != ft_nullptr)
            {
                exporter_copy = g_observability_networking_exporter;
                should_emit = true;
            }
            observability_networking_metrics_set_error(FT_ERR_SUCCESSS);
        }
    }
    ft_errno = entry_errno;
    if (guard_failed != false)
        return ;
    if (should_emit == false || exporter_copy == ft_nullptr)
        return ;
    if (exported_sample.error_tag == ft_nullptr)
    {
        if (exported_sample.error_code == FT_ERR_SUCCESSS)
            exported_sample.error_tag = "ok";
        else
            exported_sample.error_tag = ft_strerror(exported_sample.error_code);
    }
    if (exported_sample.success == false && exported_sample.error_code == FT_ERR_SUCCESSS)
        exported_sample.error_code = FT_ERR_INTERNAL;
    exporter_copy(exported_sample);
    return ;
}
