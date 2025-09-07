#ifndef TIME_HPP
# define TIME_HPP

#include <ctime>

time_t  time_now(void);
long    time_now_ms(void);
void    time_local(time_t time_value, struct tm *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);

#endif
