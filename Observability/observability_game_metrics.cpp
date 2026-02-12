#include "observability_game_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/lock_guard.hpp"
#include "../Basic/basic.hpp"

static pt_mutex g_observability_game_mutex;
static bool g_observability_game_initialized = false;
static ft_game_observability_exporter g_observability_game_exporter = ft_nullptr;
static int g_observability_game_error = FT_ERR_SUCCESS;

static void observability_game_metrics_set_error(int error_code)
{
    g_observability_game_error = error_code;
    return ;
}

int observability_game_metrics_initialize(ft_game_observability_exporter exporter)
{
    int result;
    int error_code;

    if (exporter == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        observability_game_metrics_set_error(error_code);
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    result = 0;
    error_code = FT_ERR_SUCCESS;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            error_code = guard.get_error();
            observability_game_metrics_set_error(error_code);
            result = -1;
        }
        else
        {
            g_observability_game_exporter = exporter;
            g_observability_game_initialized = true;
            error_code = FT_ERR_SUCCESS;
            observability_game_metrics_set_error(error_code);
        }
    }
    ft_global_error_stack_push(error_code);
    return (result);
}

int observability_game_metrics_shutdown(void)
{
    int result;
    int error_code;

    result = 0;
    error_code = FT_ERR_SUCCESS;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            error_code = guard.get_error();
            observability_game_metrics_set_error(error_code);
            result = -1;
        }
        else
        {
            g_observability_game_initialized = false;
            g_observability_game_exporter = ft_nullptr;
            error_code = FT_ERR_SUCCESS;
            observability_game_metrics_set_error(error_code);
        }
    }
    ft_global_error_stack_push(error_code);
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
    bool guard_failed;
    int error_code;

    if (sample.labels.event_name == ft_nullptr
        || sample.labels.entity == ft_nullptr
        || sample.labels.attribute == ft_nullptr
        || sample.unit == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        observability_game_metrics_set_error(error_code);
        ft_global_error_stack_push(error_code);
        return ;
    }
    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = false;
    guard_failed = false;
    error_code = FT_ERR_SUCCESS;
    {
        ft_lock_guard<pt_mutex> guard(g_observability_game_mutex);
        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            error_code = guard.get_error();
            observability_game_metrics_set_error(error_code);
            guard_failed = true;
        }
        else
        {
            if (g_observability_game_initialized && g_observability_game_exporter != ft_nullptr)
            {
                exporter_copy = g_observability_game_exporter;
                should_emit = true;
            }
            error_code = FT_ERR_SUCCESS;
            observability_game_metrics_set_error(error_code);
        }
    }
    if (guard_failed != false)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (should_emit == false || exporter_copy == ft_nullptr)
    {
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (exported_sample.error_tag == ft_nullptr)
    {
        if (exported_sample.error_code == FT_ERR_SUCCESS)
            exported_sample.error_tag = "ok";
        else
            exported_sample.error_tag = ft_strerror(exported_sample.error_code);
    }
    if (exported_sample.success == false && exported_sample.error_code == FT_ERR_SUCCESS)
        exported_sample.error_code = FT_ERR_INTERNAL;
    exporter_copy(exported_sample);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}
