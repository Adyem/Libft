#ifndef LOGGER_SYSLOG_HPP
#define LOGGER_SYSLOG_HPP

void ft_syslog_sink(const char *message, void *user_data);
int ft_log_set_syslog(const char *identifier);

#endif
