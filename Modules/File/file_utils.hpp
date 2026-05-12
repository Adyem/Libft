#ifndef FILE_FILE_UTILS_HPP
# define FILE_FILE_UTILS_HPP

#include "../CPP_class/class_string.hpp"
#include <cstddef>
#include <cstdio>

int32_t       file_copy(const char *source_path, const char *destination_path);
int32_t       file_copy_with_buffer(const char *source_path, const char *destination_path, ft_size_t buffer_size);
ft_size_t    file_default_copy_buffer_size(void) noexcept;
int32_t       file_move(const char *source_path, const char *destination_path);
int32_t       file_exists(const char *path);
int32_t       file_delete(const char *path);
ft_string file_path_join(const char *path_left, const char *path_right);
ft_string file_path_normalize(const char *path);
ft_bool   file_path_is_absolute(const char *path) noexcept;
ft_bool   file_path_is_relative(const char *path) noexcept;
char      *file_path_basename(const char *path);
char      *file_path_dirname(const char *path);
char      *file_path_extension(const char *path);
char      *file_path_stem(const char *path);
ft_string *file_path_basename_string(const char *path);
ft_string *file_path_dirname_string(const char *path);
ft_string *file_path_extension_string(const char *path);
ft_string *file_path_stem_string(const char *path);

FILE      *ft_fopen(const char *filename, const char *mode);
int32_t        ft_fclose(FILE *stream);
char       *ft_fgets(char *string, int32_t size, FILE *stream);

#endif
