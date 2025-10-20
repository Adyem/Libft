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

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"

class pt_mutex;

typedef struct s_su_environment_snapshot
{
    ft_vector<ft_string> entries;
}   t_su_environment_snapshot;

char    *su_getenv(const char *name);
int     su_setenv(const char *name, const char *value, int overwrite);
int     su_putenv(char *string);
int     su_environment_snapshot_capture(t_su_environment_snapshot *snapshot);
int     su_environment_snapshot_restore(const t_su_environment_snapshot *snapshot);
void    su_environment_snapshot_dispose(t_su_environment_snapshot *snapshot);
int     su_environment_sandbox_begin(t_su_environment_snapshot *snapshot);
int     su_environment_sandbox_end(t_su_environment_snapshot *snapshot);
char    *su_get_home_directory(void);
int     su_open(const char *path_name);
int     su_open(const char *path_name, int flags);
int     su_open(const char *path_name, int flags, mode_t mode);
ssize_t su_read(int file_descriptor, void *buffer, size_t count);
ssize_t su_write(int file_descriptor, const void *buffer, size_t count);
typedef ssize_t (*t_su_write_syscall_hook)(int file_descriptor, const void *buffer, size_t count);
void    su_set_write_syscall_hook(t_su_write_syscall_hook hook);
void    su_reset_write_syscall_hook(void);
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

typedef void (*t_su_resource_tracer)(const char *reason);
int     su_register_resource_tracer(t_su_resource_tracer tracer);
int     su_unregister_resource_tracer(t_su_resource_tracer tracer);
void    su_clear_resource_tracers(void);
void    su_run_resource_tracers(const char *reason);
void    su_internal_set_abort_reason(const char *reason);
const char *su_internal_take_abort_reason(void);

int     su_locale_compare(const char *left, const char *right, const char *locale_name, int *result);
int     su_locale_casefold(const char *input, const char *locale_name, ft_string &output);

typedef struct su_file
{
    pt_mutex    *mutex;
    bool        thread_safe_enabled;
    bool        closed;
    int         _descriptor;
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

int     su_file_prepare_thread_safety(su_file *stream);
void    su_file_teardown_thread_safety(su_file *stream);
int     su_file_lock(su_file *stream, bool *lock_acquired);
void    su_file_unlock(su_file *stream, bool lock_acquired);

int     su_copy_file(const char *source_path, const char *destination_path);
int     su_copy_directory_recursive(const char *source_path, const char *destination_path);
int     su_inspect_permissions(const char *path, mode_t *permissions_out);

typedef void (*t_su_service_signal_handler)(int signal_number, void *user_context);
void    su_service_force_no_fork(bool enable);
int     su_service_daemonize(const char *working_directory, const char *pid_file_path, bool redirect_standard_streams);
int     su_service_install_signal_handlers(t_su_service_signal_handler handler, void *user_context);
void    su_service_clear_signal_handlers(void);

typedef int (*t_su_health_check)(void *context, ft_string &detail);

typedef struct s_su_health_check_result
{
    ft_string   name;
    bool        healthy;
    ft_string   detail;
    int         error_code;
}   t_su_health_check_result;

int     su_health_register_check(const char *name, t_su_health_check check, void *context);
int     su_health_unregister_check(const char *name);
void    su_health_clear_checks(void);
int     su_health_run_checks(t_su_health_check_result *results, size_t capacity, size_t *count);
int     su_health_run_check(const char *name, t_su_health_check_result *result);

#endif
