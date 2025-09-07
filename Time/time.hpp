#ifndef TIME_HPP
# define TIME_HPP

#include <ctime>

time_t  ft_time_now(void);
long    ft_time_now_ms(void);
void    ft_time_local(time_t time_value, struct tm *out);
void    ft_sleep(unsigned int seconds);
void    ft_sleep_ms(unsigned int milliseconds);

#endif
