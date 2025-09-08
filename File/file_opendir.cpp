#include <unistd.h>
#include <fcntl.h>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "file_open_dir.hpp"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/syscall.h>
#include <dirent.h>
#endif

#ifdef _WIN32
static inline file_dir* opendir_win(const char* directoryPath)
{
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", directoryPath);
    file_dir* directoryStream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directoryStream)
        return (ft_nullptr);
    ft_memset(directoryStream, 0, sizeof(file_dir));
    HANDLE hFind = FindFirstFileA(searchPath, &directoryStream->w_findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cma_free(directoryStream);
        return (ft_nullptr);
    }
    directoryStream->fd = reinterpret_cast<intptr_t>(hFind);
    directoryStream->first_read = true;
    return (directoryStream);
}

static inline file_dirent* readdir_win(file_dir* dir)
{
    WIN32_FIND_DATAA* fd = &dir->w_findData;
    if (dir->first_read)
        dir->first_read = false;
    else
    {
        if (!FindNextFileA(reinterpret_cast<HANDLE>(dir->fd), fd))
            return (ft_nullptr);
    }
    static file_dirent entry;
    ft_bzero(&entry, sizeof(entry));
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(reinterpret_cast<HANDLE>(dir->fd), &info))
        entry.d_ino = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
    ft_strncpy(entry.d_name, fd->cFileName, sizeof(entry.d_name) - 1);
    if (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        entry.d_type = DT_DIR;
    else
        entry.d_type = DT_REG;
    return (&entry);
}

static inline int closedir_win(file_dir* directoryStream)
{
    FindClose(reinterpret_cast<HANDLE>(directoryStream->fd));
    cma_free(directoryStream);
    return (0);
}
#else
#ifdef __linux__
static inline file_dir* opendir_unix(const char* directoryPath)
{
    int fileDescriptor = ft_open(directoryPath, O_DIRECTORY | O_RDONLY, 0);
    if (fileDescriptor < 0)
        return (ft_nullptr);
    file_dir* directoryStream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directoryStream)
    {
        ft_close(fileDescriptor);
        return (ft_nullptr);
    }
    ft_memset(directoryStream, 0, sizeof(file_dir));
    directoryStream->fd = static_cast<intptr_t>(fileDescriptor);
    directoryStream->buffer_size = 4096;
    directoryStream->buffer = reinterpret_cast<char*>(cma_malloc(directoryStream->buffer_size));
    if (!directoryStream->buffer)
    {
        cma_free(directoryStream);
        ft_close(fileDescriptor);
        return (ft_nullptr);
    }
    directoryStream->buffer_used   = 0;
    directoryStream->buffer_offset = 0;
    return (directoryStream);
}
#else
static inline file_dir* opendir_unix(const char* directoryPath)
{
    DIR* dir = opendir(directoryPath);
    if (!dir)
        return (ft_nullptr);
    file_dir* directoryStream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directoryStream)
    {
        closedir(dir);
        return (ft_nullptr);
    }
    ft_memset(directoryStream, 0, sizeof(file_dir));
    directoryStream->fd = reinterpret_cast<intptr_t>(dir);
    return (directoryStream);
}
#endif

#ifdef __linux__
static inline file_dirent* readdir_unix(file_dir* dir)
{
    if (dir->buffer_offset >= static_cast<size_t>(dir->buffer_used))
    {
        dir->buffer_offset = 0;
        long n = syscall(SYS_getdents64, static_cast<int>(dir->fd),
                         reinterpret_cast<linux_dirent64*>(dir->buffer), dir->buffer_size);
        if (n <= 0)
            return (ft_nullptr);
        dir->buffer_used = n;
    }
    linux_dirent64* raw = reinterpret_cast<linux_dirent64*>(dir->buffer + dir->buffer_offset);
    if (raw->d_reclen == 0)
        return (ft_nullptr);
    static file_dirent entry;
    ft_bzero(&entry, sizeof(entry));
    entry.d_ino  = raw->d_ino;
    entry.d_type = raw->d_type;
    ft_strncpy(entry.d_name, raw->d_name, sizeof(entry.d_name) - 1);
    dir->buffer_offset += raw->d_reclen;
    return (&entry);
}
#else
static inline file_dirent* readdir_unix(file_dir* dir)
{
    DIR* d = reinterpret_cast<DIR*>(dir->fd);
    struct dirent* entry = readdir(d);
    if (!entry)
        return (ft_nullptr);
    static file_dirent ft_entry;
    ft_bzero(&ft_entry, sizeof(ft_entry));
    ft_entry.d_ino = entry->d_ino;
    ft_entry.d_type = entry->d_type;
    ft_strncpy(ft_entry.d_name, entry->d_name, sizeof(ft_entry.d_name) - 1);
    return (&ft_entry);
}
#endif

#ifdef __linux__
static inline int closedir_unix(file_dir* directoryStream)
{
    ft_close(static_cast<int>(directoryStream->fd));
    cma_free(directoryStream->buffer);
    cma_free(directoryStream);
    return (0);
}
#else
static inline int closedir_unix(file_dir* directoryStream)
{
    DIR* d = reinterpret_cast<DIR*>(directoryStream->fd);
    closedir(d);
    cma_free(directoryStream);
    return (0);
}
#endif
#endif

file_dir* file_opendir(const char* directoryPath)
{
#ifdef _WIN32
    return (opendir_win(directoryPath));
#else
    return (opendir_unix(directoryPath));
#endif
}

file_dirent* file_readdir(file_dir* dir)
{
    if (!dir)
        return (ft_nullptr);
#ifdef _WIN32
    return (readdir_win(dir));
#else
    return (readdir_unix(dir));
#endif
}

int file_closedir(file_dir* directoryStream)
{
    if (!directoryStream)
        return (-1);
#ifdef _WIN32
    return (closedir_win(directoryStream));
#else
    return (closedir_unix(directoryStream));
#endif
}
