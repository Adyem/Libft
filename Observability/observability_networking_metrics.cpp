#include "observability_networking_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include <new>

static pt_mutex *g_observability_networking_mutex = ft_nullptr;
static bool g_observability_networking_initialized = false;
static ft_networking_observability_exporter g_observability_networking_exporter = ft_nullptr;

int observability_networking_metrics_initialize(ft_networking_observability_exporter exporter)
{
    int result;
    int lock_result;
    int unlock_result;

    if (exporter == ft_nullptr)
        return (-1);
    if (g_observability_networking_mutex == ft_nullptr)
        return (-1);
    result = 0;
    lock_result = g_observability_networking_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    g_observability_networking_exporter = exporter;
    g_observability_networking_initialized = true;
    unlock_result = g_observability_networking_mutex->unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        result = -1;
    return (result);
}

int observability_networking_metrics_shutdown(void)
{
    int result;
    int lock_result;
    int unlock_result;

    result = 0;
    if (g_observability_networking_mutex == ft_nullptr)
        return (result);
    lock_result = g_observability_networking_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    g_observability_networking_initialized = false;
    g_observability_networking_exporter = ft_nullptr;
    unlock_result = g_observability_networking_mutex->unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        result = -1;
    return (result);
}

void observability_networking_metrics_record(const ft_networking_observability_sample &sample)
{
    ft_networking_observability_sample exported_sample;
    ft_networking_observability_exporter exporter_copy;
    bool should_emit;
    int lock_result;
    int unlock_result;

    exported_sample = sample;
    exporter_copy = ft_nullptr;
    should_emit = false;
    if (g_observability_networking_mutex == ft_nullptr)
        return ;
    lock_result = g_observability_networking_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (g_observability_networking_initialized && g_observability_networking_exporter != ft_nullptr)
    {
        exporter_copy = g_observability_networking_exporter;
        should_emit = true;
    }
    unlock_result = g_observability_networking_mutex->unlock();
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

int observability_networking_metrics_enable_thread_safety(void)
{
    if (g_observability_networking_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_mutex *mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int initialization_result = mutex_pointer->initialize();
    if (initialization_result != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialization_result);
    }
    g_observability_networking_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int observability_networking_metrics_disable_thread_safety(void)
{
    if (g_observability_networking_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    g_observability_networking_initialized = false;
    g_observability_networking_exporter = ft_nullptr;
    int destroy_result = g_observability_networking_mutex->destroy();
    delete g_observability_networking_mutex;
    g_observability_networking_mutex = ft_nullptr;
    if (destroy_result != FT_ERR_SUCCESS && destroy_result != FT_ERR_INVALID_STATE)
        return (destroy_result);
    return (FT_ERR_SUCCESS);
}
