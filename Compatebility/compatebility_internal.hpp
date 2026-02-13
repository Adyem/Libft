#ifndef COMPATEBILITY_INTERNAL_HPP
#define COMPATEBILITY_INTERNAL_HPP

#include "../File/open_dir.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include "../PThread/mutex.hpp"
#include <sys/types.h>
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
# include <BaseTsd.h>
# include <sys/stat.h>
# include <windows.h>
struct timeval
{
    int64_t tv_sec;
    int64_t tv_usec;
};
# ifndef O_DIRECTORY
#  define O_DIRECTORY 0
# endif
struct file_dir
{
    intptr_t fd;
    WIN32_FIND_DATAA w_find_data;
    bool first_read;
    pt_mutex mutex;
    bool mutex_initialized;
    file_dirent entry;
    bool closed;
};
int32_t cmp_open(const char *path_name);
int32_t cmp_open(const char *path_name, int32_t flags);
int32_t cmp_open(const char *path_name, int32_t flags, int32_t mode);
int32_t cmp_read(int32_t file_descriptor, void *buffer, ft_size_t count,
    int64_t *bytes_read_out);
int32_t cmp_write(int32_t file_descriptor, const void *buffer, ft_size_t count,
    int64_t *bytes_written_out);
#else
# include <fcntl.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/time.h>
struct file_dir
{
    intptr_t fd;
    char *buffer;
    ft_size_t buffer_size;
    int64_t buffer_used;
    ft_size_t buffer_offset;
    pt_mutex mutex;
    bool mutex_initialized;
    file_dirent entry;
    bool closed;
};
int32_t cmp_open(const char *path_name);
int32_t cmp_open(const char *path_name, int32_t flags);
int32_t cmp_open(const char *path_name, int32_t flags, mode_t mode);
int32_t cmp_read(int32_t file_descriptor, void *buffer, ft_size_t count,
    int64_t *bytes_read_out);
int32_t cmp_write(int32_t file_descriptor, const void *buffer, ft_size_t count,
    int64_t *bytes_written_out);
#endif
int32_t cmp_close(int32_t file_descriptor);
void cmp_initialize_standard_file_descriptors();

file_dir *cmp_dir_open(const char *directory_path, int32_t *error_code_out);
file_dirent *cmp_dir_read(file_dir *directory_stream, int32_t *error_code_out);
int32_t cmp_dir_close(file_dir *directory_stream, int32_t *error_code_out);
int32_t cmp_directory_exists(const char *path, int32_t *exists_out,
    int32_t *error_code_out);
char cmp_path_separator(void);
void cmp_normalize_slashes(char *data);
int32_t cmp_file_exists(const char *path, int32_t *exists_out,
    int32_t *error_code_out);
int32_t cmp_file_delete(const char *path, int32_t *error_code_out);
int32_t cmp_file_move(const char *source_path, const char *destination_path, int32_t *error_code_out);
int32_t cmp_file_copy(const char *source_path, const char *destination_path, int32_t *error_code_out);
int32_t cmp_file_create_directory(const char *path, mode_t mode, int32_t *error_code_out);
int32_t cmp_file_get_permissions(const char *path, mode_t *mode_out, int32_t *error_code_out);
int32_t cmp_file_set_permissions(const char *path, int32_t owner_permissions,
    int32_t group_permissions, int32_t other_permissions, int32_t *error_code_out);

int32_t cmp_thread_equal(pthread_t thread1, pthread_t thread2);
int32_t cmp_thread_cancel(pthread_t thread);
int32_t cmp_thread_yield(void);
int32_t cmp_thread_sleep(uint32_t milliseconds);
int32_t cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value);
int32_t cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address);

int32_t cmp_readline_enable_raw_mode(void);
int32_t cmp_readline_disable_raw_mode(void);
int32_t cmp_readline_terminal_width(int32_t *width_out);
int32_t cmp_readline_terminal_dimensions(unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels);

int32_t cmp_rng_secure_bytes(unsigned char *buffer, ft_size_t length);

int32_t cmp_secure_memzero(void *buffer, ft_size_t length);

int32_t cmp_setenv(const char *name, const char *value, int32_t overwrite);
int32_t cmp_unsetenv(const char *name);
int32_t cmp_putenv(char *string);
char **cmp_get_environ_entries(void);
const char *cmp_system_strerror(int32_t error_code);
int32_t cmp_map_system_error_to_ft(int32_t error_code);
int32_t cmp_decode_errno_offset_error(int32_t error_code);
char *cmp_get_home_directory(void);
uint32_t cmp_get_cpu_count(void);
int32_t cmp_get_total_memory(uint64_t *total_memory);
std::time_t cmp_timegm(std::tm *time_pointer);
int32_t cmp_localtime(const std::time_t *time_value, std::tm *output);
int32_t cmp_time_get_time_of_day(struct timeval *time_value);
int32_t cmp_high_resolution_time(int64_t *nanoseconds_out);
const char *cmp_service_null_device_path(void);
int32_t cmp_service_format_pid_line(char *buffer, ft_size_t buffer_size, ft_size_t *length_out);

int32_t cmp_su_write(int32_t file_descriptor, const char *buffer,
    ft_size_t length, int64_t *bytes_written_out);
int32_t cmp_socket_send_all(intptr_t socket_handle, const void *buffer,
                            ft_size_t length, int32_t flags,
                            int64_t *bytes_sent_out);

int32_t cmp_syslog_open(const char *identifier);
void cmp_syslog_write(const char *message);
void cmp_syslog_close(void);

#endif
