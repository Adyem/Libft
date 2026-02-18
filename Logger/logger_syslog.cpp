#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"

int ft_log_set_syslog(const char *identifier)
{
    if (cmp_syslog_open(identifier) != 0)
    {
        return (-1);
    }
    if (ft_log_add_sink(ft_syslog_sink, ft_nullptr) != 0)
    {
        int error_code;

        cmp_syslog_close();
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (0);
}

void ft_syslog_sink(const char *message, void *user_data)
{
    (void)user_data;
    cmp_syslog_write(message);
    return ;
}
