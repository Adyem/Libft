#include "observability_game_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/lock_guard.hpp"
#include "../Libft/libft.hpp"

static pt_mutex g_observability_game_mutex;
static bool g_observability_game_initialized = false;
static ft_game_observability_exporter g_observability_game_exporter = ft_nullptr;
static int g_observability_game_error = FT_ERR_SUCCESSS;

static void observability_game_metrics_set_error(int error_code)
{
    g_observability_game_error = error_code;
    ft_errno = error_code;
    return ;
}

int observability_game_metrics_initialize(ft_game_observability_exporter exporter)
{
    int entry_errno;
    int result;

    if (exporter == ft_nullptr)
    {
        observability_game_metrics_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    entry_errno = ft_errno;
    result = 0;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_game_metrics_set_error(guard.get_error());
            result = -1;
        }
        else
        {
            g_observability_game_exporter = exporter;
            g_observability_game_initialized = true;
            observability_game_metrics_set_error(FT_ERR_SUCCESSS);
        }
    }
    ft_errno = entry_errno;
    return (result);
}

int observability_game_metrics_shutdown(void)
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = 0;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_game_metrics_set_error(guard.get_error());
            result = -1;
        }
        else
        {
            g_observability_game_initialized = false;
            g_observability_game_exporter = ft_nullptr;
            observability_game_metrics_set_error(FT_ERR_SUCCESSS);
        }
    }
    ft_errno = entry_errno;
    return (result);
}

int observability_game_metrics_get_error(void)
{
    return (g_observability_game_error);
}

const char *observability_game_metrics_get_error_str(void)
{
    return (ft_strerror(g_observability_game_error));
}

void observability_game_metrics_record(const ft_game_observability_sample &sample)
{
    ft_game_observability_sample exported_sample;
    ft_game_observability_exporter exporter_copy;
    bool should_emit;
    int entry_errno;
    bool guard_failed;

    entry_errno = ft_errno;
    if (sample.labels.event_name == ft_nullptr
        || sample.labels.entity == ft_nullptr
        || sample.labels.attribute == ft_nullptr
        || sample.unit == ft_nullptr)
    {
        observability_game_metrics_set_error(FT_ERR_INVALID_ARGUMENT);
        ft_errno = entry_errno;
        return ;
    }
    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = false;
    guard_failed = false;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            observability_game_metrics_set_error(guard.get_error());
            guard_failed = true;
        }
        else
        {
            if (g_observability_game_initialized && g_observability_game_exporter != ft_nullptr)
            {
                exporter_copy = g_observability_game_exporter;
                should_emit = true;
            }
            observability_game_metrics_set_error(FT_ERR_SUCCESSS);
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
