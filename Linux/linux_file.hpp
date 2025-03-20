#ifndef FILE_H
# define FILE_H

# include "../Windows/windows_file.hpp"

# if defined(__linux__) || defined(__APPLE__)

# define MAX_RETRIES 10
# define RETRY_DELAY_MS 500

# include <fcntl.h>
# include <unistd.h>

void	ft_initialize_standard_file_descriptors();
ssize_t ft_write(int fd, const void *buffer, size_t count);
ssize_t ft_read(int fd, void *buf, size_t count);
int		ft_close(int fd);
int		ft_open(const char* pathname);
int 	ft_open(const char* pathname, int flags);
int 	ft_open(const char* pathname, int flags, mode_t mode);

static inline void use_dummy(void)
{
    ft_dummy();
}

# endif

#endif
