#ifndef WINDOWS_HPP
# define WINDOWS_HPP

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#ifndef O_DIRECTORY
# define O_DIRECTORY 0
#endif

#ifndef DEBUG
# define DEBUG 0
#endif

int		ft_open(const char *pathname, int flags, int mode); 
ssize_t	ft_read(int fd, void *buf, unsigned int count);
ssize_t	ft_write(int fd, const void *buf, unsigned int count);
int		ft_close(int fd);
void 	ft_initialize_standard_file_descriptors();

#endif

void	ft_dummy(void);

#endif
