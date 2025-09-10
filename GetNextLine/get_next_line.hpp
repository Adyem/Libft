#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

#include <istream>
#include <cstddef>

char    *ft_strjoin_gnl(char *string_one, char *string_two);
char    *get_next_line(std::istream &input, std::size_t buffer_size);
char    **ft_read_file_lines(std::istream &input, std::size_t buffer_size);
char    **ft_open_and_read_file(const char *file_name, std::size_t buffer_size);

#endif
