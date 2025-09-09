#ifndef SYSTEM_UTILS_HPP
# define SYSTEM_UTILS_HPP

char    *su_getenv_thread_safe(const char *name);
int     su_setenv_thread_safe(const char *name, const char *value, int overwrite);
int     su_putenv_thread_safe(char *string);
void    su_assert(bool condition, const char *message);

#endif
