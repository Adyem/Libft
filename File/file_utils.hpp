#ifndef FILE_FILE_UTILS_HPP
# define FILE_FILE_UTILS_HPP

#include "../CPP_class/class_string_class.hpp"

int       file_copy(const char *source_path, const char *destination_path);
int       file_move(const char *source_path, const char *destination_path);
int       file_exists(const char *path);
int       file_delete(const char *path);
ft_string file_path_join(const char *path_left, const char *path_right);
ft_string file_path_normalize(const char *path);

#endif
