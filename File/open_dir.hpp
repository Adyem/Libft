#ifndef FILE_OPEN_DIR_HPP
# define FILE_OPEN_DIR_HPP

#include <stdint.h>
#include <sys/types.h>

struct file_dirent
{
    uint64_t d_ino;
    char d_name[256];
    unsigned char d_type;
};

struct file_dir;

file_dir *file_opendir(const char *directory_path);
int file_closedir(file_dir *directory_stream);
file_dirent *file_readdir(file_dir *directory_stream);

int file_dir_exists(const char *rel_path);
int file_create_directory(const char *path, mode_t mode);

#endif
