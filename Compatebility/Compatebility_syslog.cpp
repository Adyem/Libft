#include "compatebility_internal.hpp"
#if defined(_WIN32) || defined(_WIN64)
int32_t cmp_syslog_open(const char *identifier)
{
    (void)identifier;
    return (-1);
}

void cmp_syslog_write(const char *message)
{
    (void)message;
    return ;
}

void cmp_syslog_close(void)
{
    return ;
}
#else
#include <syslog.h>
int32_t cmp_syslog_open(const char *identifier)
{
    openlog(identifier, LOG_PID | LOG_CONS, LOG_USER);
    return (0);
}

void cmp_syslog_write(const char *message)
{
    syslog(LOG_INFO, "%s", message);
    return ;
}

void cmp_syslog_close(void)
{
    closelog();
    return ;
}
#endif
