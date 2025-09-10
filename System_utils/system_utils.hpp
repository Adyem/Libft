#ifndef SYSTEM_UTILS_HPP
# define SYSTEM_UTILS_HPP

char    *su_getenv(const char *name);
int     su_setenv(const char *name, const char *value, int overwrite);
int     su_putenv(char *string);
unsigned int    su_get_cpu_count(void);
unsigned long long su_get_total_memory(void);
void    su_abort(void);
void    su_sigabrt(void);
void    su_sigfpe(void);
void    su_sigill(void);
void    su_sigint(void);
void    su_sigsegv(void);
void    su_sigterm(void);
void    su_assert(bool condition, const char *message);

#endif
