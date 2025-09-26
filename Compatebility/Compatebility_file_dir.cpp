#include "compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include "../CPP_class/class_nullptr.hpp"
# include <windows.h>
# ifndef DT_DIR
#  define DT_DIR 4
# endif
# ifndef DT_REG
#  define DT_REG 8
# endif

file_dir *cmp_dir_open(const char *directory_path)
{
    WIN32_FIND_DATAA find_data;
    HANDLE handle = FindFirstFileA(directory_path, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
        return (ft_nullptr);
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        FindClose(handle);
        return (ft_nullptr);
    }
    ft_memset(directory_stream, 0, sizeof(file_dir));
    directory_stream->fd = reinterpret_cast<intptr_t>(handle);
    directory_stream->w_find_data = find_data;
    directory_stream->first_read = true;
    return (directory_stream);
}

file_dirent *cmp_dir_read(file_dir *directory_stream)
{
    HANDLE handle = reinterpret_cast<HANDLE>(directory_stream->fd);
    static file_dirent entry;
    if (directory_stream->first_read)
    {
        directory_stream->first_read = false;
    }
    else if (!FindNextFileA(handle, &directory_stream->w_find_data))
        return (ft_nullptr);
    ft_bzero(&entry, sizeof(entry));
    ft_strncpy(entry.d_name, directory_stream->w_find_data.cFileName,
        sizeof(entry.d_name) - 1);
    entry.d_ino = 0;
    if (directory_stream->w_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        entry.d_type = DT_DIR;
    else
        entry.d_type = DT_REG;
    return (&entry);
}

int cmp_dir_close(file_dir *directory_stream)
{
    FindClose(reinterpret_cast<HANDLE>(directory_stream->fd));
    cma_free(directory_stream);
    return (0);
}

int cmp_directory_exists(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return (1);
    return (0);
}

#else
# include "../CPP_class/class_nullptr.hpp"
# include <dirent.h>
# include <sys/syscall.h>
# include <cstdio>
# include <stdint.h>
#ifdef __linux__
struct linux_dirent64
{
    uint64_t d_ino;
    int64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};
#endif

file_dir *cmp_dir_open(const char *directory_path)
{
#ifdef __linux__
    int file_descriptor = cmp_open(directory_path, O_DIRECTORY | O_RDONLY, 0);
    if (file_descriptor < 0)
        return (ft_nullptr);
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        cmp_close(file_descriptor);
        return (ft_nullptr);
    }
    ft_memset(directory_stream, 0, sizeof(file_dir));
    directory_stream->fd = static_cast<intptr_t>(file_descriptor);
    directory_stream->buffer_size = 4096;
    directory_stream->buffer = reinterpret_cast<char*>(cma_malloc(directory_stream->buffer_size));
    if (!directory_stream->buffer)
    {
        cma_free(directory_stream);
        cmp_close(file_descriptor);
        return (ft_nullptr);
    }
    directory_stream->buffer_used = 0;
    directory_stream->buffer_offset = 0;
    return (directory_stream);
#else
    DIR *dir = opendir(directory_path);
    if (!dir)
        return (ft_nullptr);
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        closedir(dir);
        return (ft_nullptr);
    }
    ft_memset(directory_stream, 0, sizeof(file_dir));
    directory_stream->fd = reinterpret_cast<intptr_t>(dir);
    return (directory_stream);
#endif
}

file_dirent *cmp_dir_read(file_dir *directory_stream)
{
#ifdef __linux__
    if (directory_stream->buffer_offset >= static_cast<size_t>(directory_stream->buffer_used))
    {
        directory_stream->buffer_offset = 0;
        long bytes = syscall(SYS_getdents64, static_cast<int>(directory_stream->fd),
            reinterpret_cast<linux_dirent64*>(directory_stream->buffer), directory_stream->buffer_size);
        if (bytes <= 0)
            return (ft_nullptr);
        directory_stream->buffer_used = bytes;
    }
    linux_dirent64 *raw = reinterpret_cast<linux_dirent64*>(directory_stream->buffer + directory_stream->buffer_offset);
    if (raw->d_reclen == 0)
        return (ft_nullptr);
    static file_dirent entry;
    ft_bzero(&entry, sizeof(entry));
    entry.d_ino = raw->d_ino;
    entry.d_type = raw->d_type;
    ft_strncpy(entry.d_name, raw->d_name, sizeof(entry.d_name) - 1);
    directory_stream->buffer_offset += raw->d_reclen;
    return (&entry);
#else
    DIR *dir = reinterpret_cast<DIR*>(directory_stream->fd);
    struct dirent *entry = readdir(dir);
    if (!entry)
        return (ft_nullptr);
    static file_dirent ft_entry;
    ft_bzero(&ft_entry, sizeof(ft_entry));
    ft_entry.d_ino = entry->d_ino;
    ft_entry.d_type = entry->d_type;
    ft_strncpy(ft_entry.d_name, entry->d_name, sizeof(ft_entry.d_name) - 1);
    return (&ft_entry);
#endif
}

int cmp_dir_close(file_dir *directory_stream)
{
#ifdef __linux__
    cmp_close(static_cast<int>(directory_stream->fd));
    cma_free(directory_stream->buffer);
    cma_free(directory_stream);
    return (0);
#else
    DIR *dir = reinterpret_cast<DIR*>(directory_stream->fd);
    closedir(dir);
    cma_free(directory_stream);
    return (0);
#endif
}

int cmp_directory_exists(const char *path)
{
    struct stat stat_buffer;
    if (stat(path, &stat_buffer) == 0 && S_ISDIR(stat_buffer.st_mode))
        return (1);
    return (0);
}

#endif
