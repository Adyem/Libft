#ifndef FILE_FILE_UTILS_HPP
# define FILE_FILE_UTILS_HPP

int file_copy(const char *source_path, const char *destination_path);
int file_move(const char *source_path, const char *destination_path);
int file_exists(const char *path);
int file_delete(const char *path);

#endif
