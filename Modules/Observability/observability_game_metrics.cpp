#include "observability_game_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Basic/basic.hpp"

static pt_mutex *g_observability_game_mutex = ft_nullptr;
static ft_bool g_observability_game_initialised = FT_FALSE;
static ft_game_observability_exporter g_observability_game_exporter = ft_nullptr;

int32_t observability_game_metrics_enable_thread_safety(void)
{
    if (g_observability_game_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_mutex *mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int32_t initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    g_observability_game_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t observability_game_metrics_disable_thread_safety(void)
{
    if (g_observability_game_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = g_observability_game_mutex->destroy();
    delete g_observability_game_mutex;
    g_observability_game_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool observability_game_metrics_is_thread_safe(void)
{
    return (g_observability_game_mutex != ft_nullptr);
}
int32_t observability_game_metrics_initialize(ft_game_observability_exporter exporter)
{
    int32_t operation_result;
    int32_t lock_result;
    int32_t unlock_result;

    if (exporter == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    operation_result = observability_game_metrics_enable_thread_safety();
    if (operation_result != FT_ERR_SUCCESS)
        return (operation_result);
    operation_result = FT_ERR_SUCCESS;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_game_exporter = exporter;
    g_observability_game_initialised = FT_TRUE;
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        operation_result = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return (operation_result);
}

int32_t observability_game_metrics_shutdown(void)
{
    int32_t operation_result;
    int32_t lock_result;
    int32_t unlock_result;

    operation_result = FT_ERR_SUCCESS;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    g_observability_game_initialised = FT_FALSE;
    g_observability_game_exporter = ft_nullptr;
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        operation_result = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    int32_t disable_error = observability_game_metrics_disable_thread_safety();
    if (operation_result == FT_ERR_SUCCESS)
        operation_result = disable_error;
    return (operation_result);
}

void observability_game_metrics_record(const ft_game_observability_sample &sample)
{
    ft_game_observability_sample exported_sample;
    ft_game_observability_exporter exporter_copy;
    ft_bool should_emit;
    int32_t lock_result;
    int32_t unlock_result;

    if (sample.labels.event_name == ft_nullptr
        || sample.labels.entity == ft_nullptr
        || sample.labels.attribute == ft_nullptr
        || sample.unit == ft_nullptr)
        return ;
    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = FT_FALSE;
    lock_result = pt_mutex_lock_if_not_null(g_observability_game_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (g_observability_game_initialised && g_observability_game_exporter != ft_nullptr)
    {
        exporter_copy = g_observability_game_exporter;
        should_emit = FT_TRUE;
    }
    unlock_result = pt_mutex_unlock_if_not_null(g_observability_game_mutex);
    if (unlock_result != FT_ERR_SUCCESS)
        return ;
    if (should_emit == FT_FALSE || exporter_copy == ft_nullptr)
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
    if (exported_sample.success == FT_FALSE && exported_sample.error_code == FT_ERR_SUCCESS)
        exported_sample.error_code = FT_ERR_INTERNAL;
    exporter_copy(exported_sample);
    return ;
}
