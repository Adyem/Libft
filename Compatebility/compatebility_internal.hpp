#ifndef COMPATEBILITY_INTERNAL_HPP
#define COMPATEBILITY_INTERNAL_HPP

#include "../File/open_dir.hpp"
#include <cstddef>
#include <pthread.h>
#include <sys/types.h>
#include <ctime>

class ft_socket;

#if defined(_WIN32) || defined(_WIN64)
# include <BaseTsd.h>
# include <sys/stat.h>
# include <windows.h>
typedef SSIZE_T ssize_t;
# ifndef O_DIRECTORY
#  define O_DIRECTORY 0
# endif
struct file_dir
{
    intptr_t fd;
    WIN32_FIND_DATAA w_find_data;
    bool first_read;
};
int cmp_open(const char *path_name);
int cmp_open(const char *path_name, int flags);
int cmp_open(const char *path_name, int flags, int mode);
ssize_t cmp_read(int file_descriptor, void *buffer, unsigned int count);
ssize_t cmp_write(int file_descriptor, const void *buffer, unsigned int count);
#else
# include <fcntl.h>
# include <unistd.h>
# include <sys/stat.h>
struct file_dir
{
    intptr_t fd;
    char *buffer;
    size_t buffer_size;
    ssize_t buffer_used;
    size_t buffer_offset;
};
int cmp_open(const char *path_name);
int cmp_open(const char *path_name, int flags);
int cmp_open(const char *path_name, int flags, mode_t mode);
ssize_t cmp_read(int file_descriptor, void *buffer, size_t count);
ssize_t cmp_write(int file_descriptor, const void *buffer, size_t count);
#endif
int cmp_close(int file_descriptor);
void cmp_initialize_standard_file_descriptors();

file_dir *cmp_dir_open(const char *directory_path);
file_dirent *cmp_dir_read(file_dir *directory_stream);
int cmp_dir_close(file_dir *directory_stream);
int cmp_directory_exists(const char *path);
char cmp_path_separator(void);
void cmp_normalize_slashes(char *data);
int cmp_file_exists(const char *path);
int cmp_file_delete(const char *path);
int cmp_file_move(const char *source_path, const char *destination_path);
int cmp_file_copy(const char *source_path, const char *destination_path);
int cmp_file_create_directory(const char *path, mode_t mode);

int cmp_thread_equal(pthread_t thread1, pthread_t thread2);
int cmp_thread_cancel(pthread_t thread);
int cmp_thread_yield(void);
int cmp_thread_sleep(unsigned int milliseconds);

int cmp_readline_enable_raw_mode(void);
void cmp_readline_disable_raw_mode(void);
int cmp_readline_terminal_width(void);

int cmp_rng_secure_bytes(unsigned char *buffer, size_t length);

int cmp_setenv(const char *name, const char *value, int overwrite);
int cmp_unsetenv(const char *name);
int cmp_putenv(char *string);
char *cmp_get_home_directory(void);
unsigned int cmp_get_cpu_count(void);
unsigned long long cmp_get_total_memory(void);
std::time_t cmp_timegm(std::tm *time_pointer);
int cmp_localtime(const std::time_t *time_value, std::tm *output);

ssize_t cmp_su_write(int file_descriptor, const char *buffer, size_t length);
ssize_t cmp_socket_send_all(ft_socket *socket_object, const void *buffer,
                            size_t length, int flags);

int cmp_syslog_open(const char *identifier);
void cmp_syslog_write(const char *message);
void cmp_syslog_close(void);

#endif
