#include "observability_game_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Basic/basic.hpp"

static pt_mutex *g_observability_game_mutex = ft_nullptr;
static bool g_observability_game_initialized = false;
static ft_game_observability_exporter g_observability_game_exporter = ft_nullptr;

int observability_game_metrics_enable_thread_safety(void)
{
    if (g_observability_game_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_mutex *mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    g_observability_game_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int observability_game_metrics_disable_thread_safety(void)
{
    if (g_observability_game_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = g_observability_game_mutex->destroy();
    delete g_observability_game_mutex;
    g_observability_game_mutex = ft_nullptr;
    return (destroy_error);
}

bool observability_game_metrics_is_thread_safe(void)
{
    return (g_observability_game_mutex != ft_nullptr);
}
int observability_game_metrics_initialize(ft_game_observability_exporter exporter)
{
    int result;
    int lock_result;
    int unlock_result;

    if (exporter == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    result = observability_game_metrics_enable_thread_safety();
    if (result != FT_ERR_SUCCESS)
        return (result);
    result = FT_ERR_SUCCESS;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_game_exporter = exporter;
    g_observability_game_initialized = true;
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        result = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return (result);
}

int observability_game_metrics_shutdown(void)
{
    int result;
    int lock_result;
    int unlock_result;

    result = FT_ERR_SUCCESS;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_game_initialized = false;
    g_observability_game_exporter = ft_nullptr;
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        result = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    int disable_error = observability_game_metrics_disable_thread_safety();
    if (result == FT_ERR_SUCCESS)
        result = disable_error;
    return (result);
}

void observability_game_metrics_record(const ft_game_observability_sample &sample)
{
    ft_game_observability_sample exported_sample;
    ft_game_observability_exporter exporter_copy;
    bool should_emit;
    int lock_result;
    int unlock_result;

    if (sample.labels.event_name == ft_nullptr
        || sample.labels.entity == ft_nullptr
        || sample.labels.attribute == ft_nullptr
        || sample.unit == ft_nullptr)
        return ;
    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = false;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (g_observability_game_initialized && g_observability_game_exporter != ft_nullptr)
    {
        exporter_copy = g_observability_game_exporter;
        should_emit = true;
    }
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        return ;
    if (should_emit == false || exporter_copy == ft_nullptr)
    {
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
    return ;
}
