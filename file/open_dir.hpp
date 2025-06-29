#ifndef OPEN_DIR_HPP
# define OPEN_DIR_HPP

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
# include <windows.h>
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
    int     fd;
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

int 		dir_exists(const char *rel_path);

#endif
