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
int     su_close(int file_descriptor);
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

typedef struct su_file
{
    int _descriptor;
} su_file;

void    su_force_file_stream_allocation_failure(bool should_fail);
void    su_force_fread_failure(int error_code);
void    su_clear_forced_fread_failure(void);
su_file *su_fopen(const char *path_name);
su_file *su_fopen(const char *path_name, int flags);
su_file *su_fopen(const char *path_name, int flags, mode_t mode);
int     su_fclose(su_file *stream);
size_t  su_fread(void *buffer, size_t size, size_t count, su_file *stream);
size_t  su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream);
int     su_fseek(su_file *stream, long offset, int origin);
long    su_ftell(su_file *stream);

#endif
