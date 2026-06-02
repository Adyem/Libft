#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t ft_log_set_syslog(const char *identifier)
{
    if (cmp_syslog_open(identifier) != 0)
    {
        return (FT_ERR_INTERNAL);
    }
    if (ft_log_add_sink(ft_syslog_sink, ft_nullptr) != 0)
    {
        int32_t error_code_value;

        cmp_syslog_close();
        error_code_value = FT_ERR_SUCCESS;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_syslog_sink(const char *message, void *user_data)
{
    (void)user_data;
    cmp_syslog_write(message);
    return (FT_ERR_SUCCESS);
}
