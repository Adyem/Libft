#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if defined(_WIN32) || defined(_WIN64)
int32_t cmp_syslog_open(const char *identifier)
{
    (void)identifier;
    return (FT_ERR_UNSUPPORTED_TYPE);
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
    return (FT_ERR_SUCCESS);
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
