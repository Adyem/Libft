#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"

#if defined(_WIN32) || defined(_WIN64)
int cmp_syslog_open(const char *identifier)
{
    (void)identifier;
    ft_errno = FT_EINVAL;
    return (-1);
}

void cmp_syslog_write(const char *message)
{
    (void)message;
    ft_errno = FT_EINVAL;
    return ;
}

void cmp_syslog_close(void)
{
    ft_errno = FT_EINVAL;
    return ;
}
#else
#include <syslog.h>
int cmp_syslog_open(const char *identifier)
{
    openlog(identifier, LOG_PID | LOG_CONS, LOG_USER);
    ft_errno = ER_SUCCESS;
    return (0);
}

void cmp_syslog_write(const char *message)
{
    syslog(LOG_INFO, "%s", message);
    ft_errno = ER_SUCCESS;
    return ;
}

void cmp_syslog_close(void)
{
    closelog();
    ft_errno = ER_SUCCESS;
    return ;
}
#endif
