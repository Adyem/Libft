#include "compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include "../CPP_class/class_nullptr.hpp"
# include <windows.h>
# include <errno.h>
# ifndef DT_DIR
#  define DT_DIR 4
# endif
# ifndef DT_REG
#  define DT_REG 8
# endif

file_dir *cmp_dir_open(const char *directory_path)
{
    WIN32_FIND_DATAA find_data;
    if (directory_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    size_t directory_path_length = ft_strlen(directory_path);
    size_t allocation_size = directory_path_length + 3;
    char *search_path = reinterpret_cast<char*>(cma_malloc(allocation_size));
    if (!search_path)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_strlcpy(search_path, directory_path, allocation_size);
    size_t search_path_length = ft_strlen(search_path);
    if (search_path_length > 0
        && (search_path[search_path_length - 1] == '\\'
            || search_path[search_path_length - 1] == '/'))
    {
        search_path[search_path_length] = '*';
        search_path[search_path_length + 1] = '\0';
    }
    else
    {
        search_path[search_path_length] = '\\';
        search_path[search_path_length + 1] = '*';
        search_path[search_path_length + 2] = '\0';
    }
    HANDLE handle = FindFirstFileA(search_path, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
    {
        DWORD last_error = GetLastError();
        cma_free(search_path);
        if (last_error != 0)
            ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    cma_free(search_path);
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        FindClose(handle);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_memset(directory_stream, 0, sizeof(file_dir));
    directory_stream->fd = reinterpret_cast<intptr_t>(handle);
    directory_stream->w_find_data = find_data;
    directory_stream->first_read = true;
    if (pthread_mutex_init(&directory_stream->mutex, ft_nullptr) != 0)
    {
        int mutex_error;

        mutex_error = errno;
        cma_free(directory_stream);
        FindClose(handle);
        ft_errno = ft_map_system_error(mutex_error);
        return (ft_nullptr);
    }
    directory_stream->mutex_initialized = true;
    directory_stream->closed = false;
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    ft_errno = ER_SUCCESS;
    return (directory_stream);
}

file_dirent *cmp_dir_read(file_dir *directory_stream)
{
    if (directory_stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (ft_nullptr);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    HANDLE handle = reinterpret_cast<HANDLE>(directory_stream->fd);
    if (directory_stream->first_read)
    {
        directory_stream->first_read = false;
    }
    else if (!FindNextFileA(handle, &directory_stream->w_find_data))
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error == ERROR_NO_MORE_FILES)
            ft_errno = ER_SUCCESS;
        else if (last_error != 0)
            ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        pthread_mutex_unlock(&directory_stream->mutex);
        return (ft_nullptr);
    }
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    ft_strncpy(directory_stream->entry.d_name, directory_stream->w_find_data.cFileName,
        sizeof(directory_stream->entry.d_name) - 1);
    directory_stream->entry.d_ino = 0;
    if (directory_stream->w_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        directory_stream->entry.d_type = DT_DIR;
    else
        directory_stream->entry.d_type = DT_REG;
    ft_errno = ER_SUCCESS;
    pthread_mutex_unlock(&directory_stream->mutex);
    return (&directory_stream->entry);
}

int cmp_dir_close(file_dir *directory_stream)
{
    if (directory_stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    directory_stream->closed = true;
    FindClose(reinterpret_cast<HANDLE>(directory_stream->fd));
    pthread_mutex_unlock(&directory_stream->mutex);
    pthread_mutex_destroy(&directory_stream->mutex);
    directory_stream->mutex_initialized = false;
    cma_free(directory_stream);
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_directory_exists(const char *path)
{
    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    DWORD attr = GetFileAttributesA(path);
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        if (attr & FILE_ATTRIBUTE_DIRECTORY)
        {
            ft_errno = ER_SUCCESS;
            return (1);
        }
        ft_errno = ER_SUCCESS;
        return (0);
    }
    DWORD last_error = GetLastError();
    if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (0);
}

#else
# include "../CPP_class/class_nullptr.hpp"
# include <dirent.h>
# include <errno.h>
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
    if (directory_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
#ifdef __linux__
    int file_descriptor = cmp_open(directory_path, O_DIRECTORY | O_RDONLY, 0);
    if (file_descriptor < 0)
    {
        if (errno != 0)
            ft_errno = cmp_map_system_error_to_ft(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        cmp_close(file_descriptor);
        ft_errno = FT_ERR_NO_MEMORY;
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
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    directory_stream->buffer_used = 0;
    directory_stream->buffer_offset = 0;
    if (pthread_mutex_init(&directory_stream->mutex, ft_nullptr) != 0)
    {
        int mutex_error;

        mutex_error = errno;
        cma_free(directory_stream->buffer);
        cma_free(directory_stream);
        cmp_close(file_descriptor);
        ft_errno = ft_map_system_error(mutex_error);
        return (ft_nullptr);
    }
    directory_stream->mutex_initialized = true;
    directory_stream->closed = false;
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    ft_errno = ER_SUCCESS;
    return (directory_stream);
#else
    DIR *dir = opendir(directory_path);
    if (!dir)
    {
        if (errno != 0)
            ft_errno = cmp_map_system_error_to_ft(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_dir *directory_stream = reinterpret_cast<file_dir*>(cma_malloc(sizeof(file_dir)));
    if (!directory_stream)
    {
        closedir(dir);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_memset(directory_stream, 0, sizeof(file_dir));
    directory_stream->fd = reinterpret_cast<intptr_t>(dir);
    if (pthread_mutex_init(&directory_stream->mutex, ft_nullptr) != 0)
    {
        int mutex_error;

        mutex_error = errno;
        cma_free(directory_stream);
        closedir(dir);
        ft_errno = ft_map_system_error(mutex_error);
        return (ft_nullptr);
    }
    directory_stream->mutex_initialized = true;
    directory_stream->closed = false;
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    ft_errno = ER_SUCCESS;
    return (directory_stream);
#endif
}

file_dirent *cmp_dir_read(file_dir *directory_stream)
{
    if (directory_stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
#ifdef __linux__
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (ft_nullptr);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    if (directory_stream->buffer_offset >= static_cast<size_t>(directory_stream->buffer_used))
    {
        long bytes;

        directory_stream->buffer_offset = 0;
        bytes = syscall(SYS_getdents64, static_cast<int>(directory_stream->fd),
            reinterpret_cast<linux_dirent64*>(directory_stream->buffer), directory_stream->buffer_size);
        if (bytes <= 0)
        {
            if (bytes == 0)
                ft_errno = ER_SUCCESS;
            else if (errno != 0)
                ft_errno = cmp_map_system_error_to_ft(errno);
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
            pthread_mutex_unlock(&directory_stream->mutex);
            return (ft_nullptr);
        }
        directory_stream->buffer_used = bytes;
    }
    linux_dirent64 *raw = reinterpret_cast<linux_dirent64*>(directory_stream->buffer + directory_stream->buffer_offset);
    if (raw->d_reclen == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        pthread_mutex_unlock(&directory_stream->mutex);
        return (ft_nullptr);
    }
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    directory_stream->entry.d_ino = raw->d_ino;
    directory_stream->entry.d_type = raw->d_type;
    ft_strncpy(directory_stream->entry.d_name, raw->d_name, sizeof(directory_stream->entry.d_name) - 1);
    directory_stream->buffer_offset += raw->d_reclen;
    ft_errno = ER_SUCCESS;
    pthread_mutex_unlock(&directory_stream->mutex);
    return (&directory_stream->entry);
#else
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (ft_nullptr);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    DIR *dir = reinterpret_cast<DIR*>(directory_stream->fd);
    struct dirent *entry = readdir(dir);
    if (!entry)
    {
        if (errno != 0)
            ft_errno = cmp_map_system_error_to_ft(errno);
        else
            ft_errno = ER_SUCCESS;
        pthread_mutex_unlock(&directory_stream->mutex);
        return (ft_nullptr);
    }
    ft_bzero(&directory_stream->entry, sizeof(directory_stream->entry));
    directory_stream->entry.d_ino = entry->d_ino;
    directory_stream->entry.d_type = entry->d_type;
    ft_strncpy(directory_stream->entry.d_name, entry->d_name, sizeof(directory_stream->entry.d_name) - 1);
    ft_errno = ER_SUCCESS;
    pthread_mutex_unlock(&directory_stream->mutex);
    return (&directory_stream->entry);
#endif
}

int cmp_dir_close(file_dir *directory_stream)
{
#ifdef __linux__
    if (directory_stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    directory_stream->closed = true;
    cmp_close(static_cast<int>(directory_stream->fd));
    cma_free(directory_stream->buffer);
    pthread_mutex_unlock(&directory_stream->mutex);
    pthread_mutex_destroy(&directory_stream->mutex);
    directory_stream->mutex_initialized = false;
    cma_free(directory_stream);
    ft_errno = ER_SUCCESS;
    return (0);
#else
    if (directory_stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!directory_stream->mutex_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (pthread_mutex_lock(&directory_stream->mutex) != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    if (directory_stream->closed)
    {
        pthread_mutex_unlock(&directory_stream->mutex);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    directory_stream->closed = true;
    DIR *dir = reinterpret_cast<DIR*>(directory_stream->fd);
    closedir(dir);
    pthread_mutex_unlock(&directory_stream->mutex);
    pthread_mutex_destroy(&directory_stream->mutex);
    directory_stream->mutex_initialized = false;
    cma_free(directory_stream);
    ft_errno = ER_SUCCESS;
    return (0);
#endif
}

int cmp_directory_exists(const char *path)
{
    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    struct stat stat_buffer;
    if (stat(path, &stat_buffer) == 0)
    {
        if (S_ISDIR(stat_buffer.st_mode))
        {
            ft_errno = ER_SUCCESS;
            return (1);
        }
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (errno != 0)
        ft_errno = cmp_map_system_error_to_ft(errno);
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (0);
}

#endif
