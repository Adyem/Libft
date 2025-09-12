#ifndef SYSTEM_UTILS_HPP
# define SYSTEM_UTILS_HPP

#if defined(_WIN32) || defined(_WIN64)
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;
# ifndef O_DIRECTORY
#  define O_DIRECTORY 0
# endif
#else
# include <fcntl.h>
# include <unistd.h>
#endif

char    *su_getenv(const char *name);
int     su_setenv(const char *name, const char *value, int overwrite);
int     su_putenv(char *string);
char    *su_get_home_directory(void);
int     su_open(const char *path_name);
int     su_open(const char *path_name, int flags);
int     su_open(const char *path_name, int flags, mode_t mode);
ssize_t su_read(int file_descriptor, void *buffer, size_t count);
ssize_t su_write(int file_descriptor, const void *buffer, size_t count);
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
