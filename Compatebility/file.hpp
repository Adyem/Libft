#ifndef COMPATEBILITY_FILE_HPP
# define COMPATEBILITY_FILE_HPP

#if defined(_WIN32) || defined(_WIN64)
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;
# ifndef O_DIRECTORY
#  define O_DIRECTORY 0
# endif
int ft_open(const char *path_name);
int ft_open(const char *path_name, int flags);
int ft_open(const char *path_name, int flags, int mode);
ssize_t ft_read(int file_descriptor, void *buffer, unsigned int count);
ssize_t ft_write(int file_descriptor, const void *buffer, unsigned int count);
int ft_close(int file_descriptor);
void ft_initialize_standard_file_descriptors();
#else
# include <fcntl.h>
# include <unistd.h>
int ft_open(const char *path_name);
int ft_open(const char *path_name, int flags);
int ft_open(const char *path_name, int flags, mode_t mode);
ssize_t ft_read(int file_descriptor, void *buffer, size_t count);
ssize_t ft_write(int file_descriptor, const void *buffer, size_t count);
int ft_close(int file_descriptor);
void ft_initialize_standard_file_descriptors();
#endif

#endif
