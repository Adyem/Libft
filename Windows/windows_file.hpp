#ifndef WINDOWS_HPP
# define WINDOWS_HPP

#ifdef _WIN32

#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#ifndef DEBUG
# define DEBUG 0
#endif

# define O_RDONLY    0x0000
# define O_WRONLY    0x0001
# define O_RDWR      0x0002
# define O_CREAT     0x0100
# define O_EXCL      0x0200
# define O_TRUNC     0x0400
# define O_APPEND    0x0800

int		ft_open(const char *pathname, int flags, int mode); 
ssize_t	ft_read(int fd, void *buf, unsigned int count);
ssize_t	ft_write(int fd, const void *buf, unsigned int count);
int		ft_close(int fd);

#endif

void	ft_dummy(void);

#endif
