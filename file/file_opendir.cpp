#include <unistd.h>
#include <fcntl.h>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "open_dir.hpp"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/syscall.h>
#include <dirent.h>
#endif

#ifdef _WIN32
static inline FT_DIR* opendir_win(const char* directoryPath)
{
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", directoryPath);
    FT_DIR* directoryStream = reinterpret_cast<FT_DIR*>(cma_malloc(sizeof(FT_DIR)));
    if (!directoryStream)
        return (ft_nullptr);
    ft_memset(directoryStream, 0, sizeof(FT_DIR));
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

static inline ft_dirent* readdir_win(FT_DIR* dir)
{
    WIN32_FIND_DATAA* fd = &dir->w_findData;
    if (dir->first_read)
        dir->first_read = false;
    else
    {
        if (!FindNextFileA(reinterpret_cast<HANDLE>(dir->fd), fd))
            return (ft_nullptr);
    }
    static ft_dirent entry;
    ft_bzero(&entry, sizeof(entry));
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(reinterpret_cast<HANDLE>(dir->fd), &info))
        entry.d_ino = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
    ft_strncpy(entry.d_name, fd->cFileName, sizeof(entry.d_name) - 1);
    entry.d_type = (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DT_DIR : DT_REG;
    return (&entry);
}

static inline int closedir_win(FT_DIR* directoryStream)
{
    FindClose(reinterpret_cast<HANDLE>(directoryStream->fd));
    cma_free(directoryStream);
    return (0);
}
#else
#ifdef __linux__
static inline FT_DIR* opendir_unix(const char* directoryPath)
{
    int fileDescriptor = ft_open(directoryPath, O_DIRECTORY | O_RDONLY, 0);
    if (fileDescriptor < 0)
        return (ft_nullptr);
    FT_DIR* directoryStream = reinterpret_cast<FT_DIR*>(cma_malloc(sizeof(FT_DIR)));
    if (!directoryStream)
    {
        ft_close(fileDescriptor);
        return (ft_nullptr);
    }
    ft_memset(directoryStream, 0, sizeof(FT_DIR));
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
static inline FT_DIR* opendir_unix(const char* directoryPath)
{
    DIR* dir = opendir(directoryPath);
    if (!dir)
        return (ft_nullptr);
    FT_DIR* directoryStream = reinterpret_cast<FT_DIR*>(cma_malloc(sizeof(FT_DIR)));
    if (!directoryStream)
    {
        closedir(dir);
        return (ft_nullptr);
    }
    ft_memset(directoryStream, 0, sizeof(FT_DIR));
    directoryStream->fd = reinterpret_cast<intptr_t>(dir);
    return (directoryStream);
}
#endif

#ifdef __linux__
static inline ft_dirent* readdir_unix(FT_DIR* dir)
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
    static ft_dirent entry;
    ft_bzero(&entry, sizeof(entry));
    entry.d_ino  = raw->d_ino;
    entry.d_type = raw->d_type;
    ft_strncpy(entry.d_name, raw->d_name, sizeof(entry.d_name) - 1);
    dir->buffer_offset += raw->d_reclen;
    return (&entry);
}
#else
static inline ft_dirent* readdir_unix(FT_DIR* dir)
{
    DIR* d = reinterpret_cast<DIR*>(dir->fd);
    struct dirent* entry = readdir(d);
    if (!entry)
        return (ft_nullptr);
    static ft_dirent ft_entry;
    ft_bzero(&ft_entry, sizeof(ft_entry));
    ft_entry.d_ino = entry->d_ino;
    ft_entry.d_type = entry->d_type;
    ft_strncpy(ft_entry.d_name, entry->d_name, sizeof(ft_entry.d_name) - 1);
    return (&ft_entry);
}
#endif

#ifdef __linux__
static inline int closedir_unix(FT_DIR* directoryStream)
{
    ft_close(static_cast<int>(directoryStream->fd));
    cma_free(directoryStream->buffer);
    cma_free(directoryStream);
    return (0);
}
#else
static inline int closedir_unix(FT_DIR* directoryStream)
{
    DIR* d = reinterpret_cast<DIR*>(directoryStream->fd);
    closedir(d);
    cma_free(directoryStream);
    return (0);
}
#endif
#endif

FT_DIR* ft_opendir(const char* directoryPath)
{
#ifdef _WIN32
    return (opendir_win(directoryPath));
#else
    return (opendir_unix(directoryPath));
#endif
}

ft_dirent* ft_readdir(FT_DIR* dir)
{
    if (!dir)
        return (ft_nullptr);
#ifdef _WIN32
    return (readdir_win(dir));
#else
    return (readdir_unix(dir));
#endif
}

int ft_closedir(FT_DIR* directoryStream)
{
    if (!directoryStream)
        return (-1);
#ifdef _WIN32
    return (closedir_win(directoryStream));
#else
    return (closedir_unix(directoryStream));
#endif
}
