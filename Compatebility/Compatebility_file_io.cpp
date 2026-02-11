#include "compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

#if defined(_WIN32) || defined(_WIN64)
# include "../CPP_class/class_nullptr.hpp"
# include "../PThread/mutex.hpp"
# include "../PThread/pthread_internal.hpp"
# include "../Basic/basic.hpp"
# include <windows.h>
# include <stdio.h>
# include <io.h>
# include <fcntl.h>

static HANDLE g_file_handles[1024];
static pt_mutex g_file_mutex;

static int32_t cmp_store_handle(HANDLE file_handle)
{
    int32_t handle_index = 3;
    while (handle_index < 1024)
    {
        if (g_file_handles[handle_index] == ft_nullptr)
        {
            g_file_handles[handle_index] = file_handle;
            return (handle_index);
        }
        handle_index++;
    }
    return (-1);
}

static HANDLE cmp_retrieve_handle(int32_t file_descriptor)
{
    if (file_descriptor < 0 || file_descriptor >= 1024)
        return (INVALID_HANDLE_VALUE);
    return (g_file_handles[file_descriptor]);
}

static void cmp_clear_handle(int32_t file_descriptor)
{
    if (file_descriptor < 0 || file_descriptor >= 1024)
        return ;
    g_file_handles[file_descriptor] = ft_nullptr;
    return ;
}

static int32_t cmp_lock_file_mutex(void)
{
    int32_t lock_result = g_file_mutex.lock();

    if (lock_result != FT_ERR_SUCCESSS)
        return (lock_result);
    return (FT_ERR_SUCCESSS);
}

static int32_t cmp_unlock_file_mutex(void)
{
    g_file_mutex.unlock();
    return (FT_ERR_SUCCESSS);
}

static int32_t cmp_open_internal(const char *path_name, int32_t flags, int32_t mode)
{
    int32_t lock_error;

    lock_error = cmp_lock_file_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (-1);
    DWORD desired_access = 0;
    DWORD creation_disposition = 0;
    DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
    (void)mode;
    if (flags & O_DIRECTORY)
    {
        desired_access = FILE_LIST_DIRECTORY;
        creation_disposition = OPEN_EXISTING;
        flags_and_attributes = FILE_FLAG_BACKUP_SEMANTICS;
    }
    else
    {
        if ((flags & O_RDWR) == O_RDWR)
            desired_access = GENERIC_READ | GENERIC_WRITE;
        else if (flags & O_WRONLY)
            desired_access = GENERIC_WRITE;
        else
            desired_access = GENERIC_READ;
        if ((flags & O_CREAT) && (flags & O_EXCL))
            creation_disposition = CREATE_NEW;
        else if ((flags & O_CREAT) && (flags & O_TRUNC))
            creation_disposition = CREATE_ALWAYS;
        else if (flags & O_CREAT)
            creation_disposition = OPEN_ALWAYS;
        else if (flags & O_TRUNC)
            creation_disposition = TRUNCATE_EXISTING;
        else
            creation_disposition = OPEN_EXISTING;
        if (flags & O_APPEND)
            desired_access |= FILE_APPEND_DATA;
    }
    HANDLE file_handle = CreateFileA(path_name, desired_access,
        FILE_SHARE_READ | FILE_SHARE_WRITE, ft_nullptr, creation_disposition,
        flags_and_attributes, ft_nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        cmp_unlock_file_mutex();
        return (-1);
    }
    int32_t file_descriptor = cmp_store_handle(file_handle);
    if (file_descriptor < 0)
    {
        CloseHandle(file_handle);
        cmp_unlock_file_mutex();
        return (-1);
    }
    cmp_unlock_file_mutex();
    return (file_descriptor);
}

int32_t cmp_open(const char *path_name)
{
    return (cmp_open_internal(path_name, O_RDONLY, 0));
}

int32_t cmp_open(const char *path_name, int32_t flags)
{
    return (cmp_open_internal(path_name, flags, 0));
}

int32_t cmp_open(const char *path_name, int32_t flags, int32_t mode)
{
    return (cmp_open_internal(path_name, flags, mode));
}

int32_t cmp_read(int32_t file_descriptor, void *buffer, ft_size_t count,
    int64_t *bytes_read_out)
{
    int32_t lock_error;
    DWORD bytes_read;
    BOOL is_successful;
    HANDLE file_handle;
    BY_HANDLE_FILE_INFORMATION file_info;

    if (bytes_read_out != ft_nullptr)
        *bytes_read_out = 0;
    if (buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (count > static_cast<ft_size_t>(UINT32_MAX))
        return (FT_ERR_OUT_OF_RANGE);

    lock_error = cmp_lock_file_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    file_handle = cmp_retrieve_handle(file_descriptor);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        cmp_unlock_file_mutex();
        return (FT_ERR_INVALID_HANDLE);
    }
    if (GetFileInformationByHandle(file_handle, &file_info))
    {
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            cmp_unlock_file_mutex();
            return (FT_ERR_INVALID_HANDLE);
        }
    }
    bytes_read = 0;
    is_successful = ReadFile(file_handle, buffer, static_cast<DWORD>(count),
        &bytes_read, ft_nullptr);
    cmp_unlock_file_mutex();
    if (!is_successful)
    {
        return (cmp_map_system_error_to_ft(static_cast<int32_t>(GetLastError())));
    }
    if (bytes_read_out != ft_nullptr)
        *bytes_read_out = static_cast<int64_t>(bytes_read);
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_write(int32_t file_descriptor, const void *buffer, ft_size_t count,
    int64_t *bytes_written_out)
{
    int32_t lock_error;
    DWORD bytes_written;
    BOOL is_successful;
    HANDLE file_handle;

    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = 0;
    if (buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (count > static_cast<ft_size_t>(UINT32_MAX))
        return (FT_ERR_OUT_OF_RANGE);

    lock_error = cmp_lock_file_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    file_handle = cmp_retrieve_handle(file_descriptor);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        cmp_unlock_file_mutex();
        return (FT_ERR_INVALID_HANDLE);
    }
    bytes_written = 0;
    is_successful = WriteFile(file_handle, buffer, static_cast<DWORD>(count),
        &bytes_written, ft_nullptr);
    cmp_unlock_file_mutex();
    if (!is_successful)
        return (cmp_map_system_error_to_ft(static_cast<int32_t>(GetLastError())));
    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = static_cast<int64_t>(bytes_written);
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_close(int32_t file_descriptor)
{
    int32_t lock_error;
    HANDLE file_handle;
    DWORD last_error;
    int32_t error_code;

    lock_error = cmp_lock_file_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    file_handle = cmp_retrieve_handle(file_descriptor);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        cmp_unlock_file_mutex();
        return (FT_ERR_INVALID_HANDLE);
    }
    if (!CloseHandle(file_handle))
    {
        cmp_unlock_file_mutex();
        last_error = GetLastError();
        if (last_error != 0)
            error_code = cmp_map_system_error_to_ft(static_cast<int32_t>(last_error));
        else
            error_code = FT_ERR_IO;
        return (error_code);
    }
    cmp_clear_handle(file_descriptor);
    cmp_unlock_file_mutex();
    return (FT_ERR_SUCCESSS);
}

void cmp_initialize_standard_file_descriptors()
{
    static int32_t initialized = 0;
    if (initialized == 1)
        return ;
    HANDLE standard_input = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE standard_output = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE standard_error = GetStdHandle(STD_ERROR_HANDLE);
    if (standard_input != INVALID_HANDLE_VALUE)
    {
        int32_t file_descriptor_input = _open_osfhandle(
            reinterpret_cast<intptr_t>(standard_input), _O_RDONLY);
        if (file_descriptor_input != -1)
            _dup2(file_descriptor_input, 0);
    }
    if (standard_output != INVALID_HANDLE_VALUE)
    {
        int32_t file_descriptor_output = _open_osfhandle(
            reinterpret_cast<intptr_t>(standard_output), _O_WRONLY);
        if (file_descriptor_output != -1)
            _dup2(file_descriptor_output, 1);
    }
    if (standard_error != INVALID_HANDLE_VALUE)
    {
        int32_t file_descriptor_error = _open_osfhandle(
            reinterpret_cast<intptr_t>(standard_error), _O_WRONLY);
        if (file_descriptor_error != -1)
            _dup2(file_descriptor_error, 2);
    }
    int32_t lock_error;

    lock_error = cmp_lock_file_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    g_file_handles[0] = standard_input;
    g_file_handles[1] = standard_output;
    g_file_handles[2] = standard_error;
    cmp_unlock_file_mutex();
    _setmode(0, _O_BINARY);
    _setmode(1, _O_BINARY);
    _setmode(2, _O_BINARY);
    initialized = 1;
    return ;
}

#else
# include "../CPP_class/class_nullptr.hpp"
# include "../Basic/basic.hpp"
# include <fcntl.h>
# include <unistd.h>
# include <sys/stat.h>
# include <cerrno>

int32_t cmp_open(const char *path_name)
{
    int32_t file_descriptor = open(path_name, O_RDONLY);
    if (file_descriptor == -1)
    {
        return (-1);
    }
    return (file_descriptor);
}

int32_t cmp_open(const char *path_name, int32_t flags)
{
    int32_t file_descriptor = open(path_name, flags);
    if (file_descriptor == -1)
    {
        return (-1);
    }
    return (file_descriptor);
}

int32_t cmp_open(const char *path_name, int32_t flags, mode_t mode)
{
    int32_t file_descriptor = open(path_name, flags, mode);
    if (file_descriptor == -1)
    {
        return (-1);
    }
    return (file_descriptor);
}

int32_t cmp_read(int32_t file_descriptor, void *buffer, ft_size_t count,
    int64_t *bytes_read_out)
{
    int64_t bytes_read;

    if (bytes_read_out != ft_nullptr)
        *bytes_read_out = 0;
    if (buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_descriptor < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    bytes_read = read(file_descriptor, buffer, count);
    if (bytes_read == -1)
        return (cmp_map_system_error_to_ft(errno));
    if (bytes_read_out != ft_nullptr)
        *bytes_read_out = bytes_read;
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_write(int32_t file_descriptor, const void *buffer, ft_size_t count,
    int64_t *bytes_written_out)
{
    int64_t bytes_written;

    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = 0;
    if (buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_descriptor < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    bytes_written = write(file_descriptor, buffer, count);
    if (bytes_written == -1)
        return (cmp_map_system_error_to_ft(errno));
    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = bytes_written;
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_close(int32_t file_descriptor)
{
    if (file_descriptor < 0)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (close(file_descriptor) == -1)
    {
        return (cmp_map_system_error_to_ft(errno));
    }
    return (FT_ERR_SUCCESSS);
}

void cmp_initialize_standard_file_descriptors()
{
    return ;
}

#endif
