#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

#include "../Libft/libft.hpp"
#include <cstddef>

char    *ft_strjoin_gnl(char *string_one, char *string_two);
char    *get_next_line(int fd, std::size_t buffer_size);
void    gnl_set_leftover_alloc_hook(void *(*hook)(ft_size_t size));
void    gnl_reset_leftover_alloc_hook(void);
void    gnl_reset_all_streams(void);
int     gnl_clear_stream(int fd);

#endif
