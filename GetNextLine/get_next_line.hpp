#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

#include "../Basic/basic.hpp"
#include "../Template/vector.hpp"
#include <cstddef>

char    *ft_strjoin_gnl(char *string_one, char *string_two);
char    *get_next_line(int file_descriptor, std::size_t buffer_size);
void    gnl_set_leftover_alloc_hook(void *(*hook)(ft_size_t size));
void    gnl_reset_leftover_alloc_hook(void);
void    gnl_reset_all_streams(void);
int     gnl_clear_stream(int file_descriptor);
int     ft_read_file_lines(int file_descriptor, ft_vector<ft_string> &lines, std::size_t buffer_size);
int     ft_open_and_read_file(const char *path, ft_vector<ft_string> &lines, std::size_t buffer_size);

#endif
