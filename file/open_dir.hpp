#ifndef OPEN_DIR_HPP
# define OPEN_DIR_HPP

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
# include <windows.h>
#endif

#ifdef __WIN32
# ifndef DT_DIR
#  define DT_DIR 4
# endif

# ifndef DT_REG
#  define DT_REG 8
# endif
#endif

struct linux_dirent64
{
    uint64_t d_ino;
    int64_t  d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

struct ft_dirent
{
    uint64_t       d_ino;
    char           d_name[256];
    unsigned char  d_type;
};

struct FT_DIR
{
    intptr_t fd;
    char*   buffer;
    size_t  buffer_size;
    ssize_t buffer_used;
    size_t  buffer_offset;
#ifdef _WIN32
    WIN32_FIND_DATAA w_findData;
    bool             first_read;
#endif
};

FT_DIR* 	ft_opendir(const char* directoryPath);
int 		ft_closedir(FT_DIR* directoryStream);
ft_dirent	*ft_readdir(FT_DIR* directoryStream);

int         dir_exists(const char *rel_path);
int         file_create_directory(const char* path, mode_t mode);

#endif
