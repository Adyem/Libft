#include "logger_internal.hpp"

t_log_level g_level = LOG_LEVEL_INFO;
ft_vector<s_log_sink> g_sinks;
pthread_mutex_t g_sinks_mutex;
static pthread_once_t g_sinks_mutex_once = PTHREAD_ONCE_INIT;
static int32_t g_sinks_mutex_init_error = 0;
ft_bool g_use_color = FT_TRUE;

static int32_t logger_ensure_sinks_ready()
{
    uint8_t state_value;
    int32_t initialize_status;

    state_value = g_sinks.is_initialised();
    if (state_value == FT_CLASS_STATE_INITIALISED)
    {
        state_value = g_redaction_rules.is_initialised();
        if (state_value == FT_CLASS_STATE_INITIALISED)
            return (FT_ERR_SUCCESS);
        initialize_status = g_redaction_rules.initialize();
        if (initialize_status != FT_ERR_SUCCESS)
            return (initialize_status);
        return (FT_ERR_SUCCESS);
    }
    initialize_status = g_sinks.initialize();
    if (initialize_status != FT_ERR_SUCCESS)
        return (initialize_status);
    state_value = g_redaction_rules.is_initialised();
    if (state_value == FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    initialize_status = g_redaction_rules.initialize();
    if (initialize_status != FT_ERR_SUCCESS)
        return (initialize_status);
    return (FT_ERR_SUCCESS);
}

static void logger_initialize_sinks_mutex()
{
    int32_t initialization_status;

    initialization_status = pthread_mutex_init(&g_sinks_mutex, nullptr);
    if (initialization_status != 0)
    {
        g_sinks_mutex_init_error = initialization_status;
        return ;
    }
    g_sinks_mutex_init_error = 0;
    return ;
}

int32_t logger_lock_sinks()
{
    int32_t once_status;
    int32_t lock_status;
    int32_t sinks_status;

    once_status = pthread_once(&g_sinks_mutex_once, logger_initialize_sinks_mutex);
    if (once_status != 0)
        return (once_status);
    if (g_sinks_mutex_init_error != 0)
        return (g_sinks_mutex_init_error);
    lock_status = pthread_mutex_lock(&g_sinks_mutex);
    if (lock_status != 0)
        return (lock_status);
    sinks_status = logger_ensure_sinks_ready();
    if (sinks_status != FT_ERR_SUCCESS)
    {
        (void)pthread_mutex_unlock(&g_sinks_mutex);
        return (sinks_status);
    }
    return (FT_ERR_SUCCESS);
}

int32_t logger_unlock_sinks()
{
    int32_t once_status;
    int32_t unlock_status;

    once_status = pthread_once(&g_sinks_mutex_once, logger_initialize_sinks_mutex);
    if (once_status != 0)
        return (once_status);
    if (g_sinks_mutex_init_error != 0)
        return (g_sinks_mutex_init_error);
    unlock_status = pthread_mutex_unlock(&g_sinks_mutex);
    if (unlock_status != 0)
        return (unlock_status);
    return (FT_ERR_SUCCESS);
}
