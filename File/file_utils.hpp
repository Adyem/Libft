#ifndef FILE_FILE_UTILS_HPP
# define FILE_FILE_UTILS_HPP

#include "../CPP_class/class_string_class.hpp"
#include <cstddef>

int       file_copy(const char *source_path, const char *destination_path);
int       file_copy_with_buffer(const char *source_path, const char *destination_path, size_t buffer_size);
size_t    file_default_copy_buffer_size(void) noexcept;
int       file_move(const char *source_path, const char *destination_path);
int       file_exists(const char *path);
int       file_delete(const char *path);
ft_string file_path_join(const char *path_left, const char *path_right);
ft_string file_path_normalize(const char *path);

#endif
