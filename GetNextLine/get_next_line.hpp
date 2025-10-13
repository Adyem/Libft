#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

#include "../CPP_class/class_istream.hpp"
#include "../Libft/libft.hpp"
#include <cstddef>

char    *ft_strjoin_gnl(char *string_one, char *string_two);
char    *get_next_line(ft_istream &input, std::size_t buffer_size);
char    **ft_read_file_lines(ft_istream &input, std::size_t buffer_size);
char    **ft_open_and_read_file(const char *file_name, std::size_t buffer_size);
void    gnl_set_leftover_alloc_hook(void *(*hook)(ft_size_t size));
void    gnl_reset_leftover_alloc_hook(void);
int     gnl_clear_stream(ft_istream &input);

#endif
