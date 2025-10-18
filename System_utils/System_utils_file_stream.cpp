#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstdlib>
#include <cerrno>
#include <limits>

static bool g_force_file_stream_allocation_failure = false;
static int g_force_fread_failure_enabled = 0;
static int g_force_fread_failure_error = ER_SUCCESS;

int su_file_prepare_thread_safety(su_file *stream)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (stream->thread_safe_enabled == true && stream->mutex != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    stream->mutex = mutex_pointer;
    stream->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void su_file_teardown_thread_safety(su_file *stream)
{
    if (stream == ft_nullptr)
        return ;
    if (stream->mutex != ft_nullptr)
    {
        stream->mutex->~pt_mutex();
        std::free(stream->mutex);
        stream->mutex = ft_nullptr;
    }
    stream->thread_safe_enabled = false;
    return ;
}

int su_file_lock(su_file *stream, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (stream->thread_safe_enabled == false || stream->mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    stream->mutex->lock(THREAD_ID);
    if (stream->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = stream->mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void su_file_unlock(su_file *stream, bool lock_acquired)
{
    int entry_errno;

    if (stream == ft_nullptr || lock_acquired == false)
        return ;
    if (stream->mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    stream->mutex->unlock(THREAD_ID);
    if (stream->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = stream->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void su_force_fread_failure(int error_code)
{
    if (error_code == ER_SUCCESS)
    {
        g_force_fread_failure_enabled = 0;
        g_force_fread_failure_error = ER_SUCCESS;
        return ;
    }
    g_force_fread_failure_enabled = 1;
    g_force_fread_failure_error = error_code;
    return ;
}

void su_clear_forced_fread_failure(void)
{
    g_force_fread_failure_enabled = 0;
    g_force_fread_failure_error = ER_SUCCESS;
    return ;
}

void su_force_file_stream_allocation_failure(bool should_fail)
{
    g_force_file_stream_allocation_failure = should_fail;
    return ;
}

static su_file *create_file_stream(int file_descriptor)
{
    su_file *file_stream;
    bool    prepared;

    if (file_descriptor < 0)
        return (ft_nullptr);
    if (g_force_file_stream_allocation_failure == true)
    {
        cmp_close(file_descriptor);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
    {
        cmp_close(file_descriptor);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    file_stream->mutex = ft_nullptr;
    file_stream->thread_safe_enabled = false;
    file_stream->closed = false;
    file_stream->_descriptor = file_descriptor;
    prepared = (su_file_prepare_thread_safety(file_stream) == 0);
    if (prepared == false)
    {
        file_stream->_descriptor = -1;
        std::free(file_stream);
        cmp_close(file_descriptor);
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (file_stream);
}

su_file *su_fopen(const char *path_name)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags, mode_t mode)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags, mode);
    return (create_file_stream(file_descriptor));
}

int su_fclose(su_file *stream)
{
    int result;
    bool lock_acquired;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1);
    stream->closed = true;
    result = cmp_close(stream->_descriptor);
    if (result == 0)
    {
        ft_errno = ER_SUCCESS;
        su_file_unlock(stream, lock_acquired);
        su_file_teardown_thread_safety(stream);
        std::free(stream);
        return (0);
    }
    stream->closed = false;
    su_file_unlock(stream, lock_acquired);
    if (ft_errno != ER_SUCCESS)
    {
        ft_errno = cmp_normalize_ft_errno(ft_errno);
        return (result);
    }
    if (errno != 0)
        ft_errno = cmp_map_system_error_to_ft(errno);
    return (result);
}

size_t su_fread(void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t total_size;
    size_t total_read;
    char *byte_buffer;
    ssize_t bytes_read;
    size_t maximum_size;
    bool lock_acquired;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (size == 0 || count == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (0);
    if (stream->closed == true)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    if (g_force_fread_failure_enabled != 0)
    {
        int forced_error;

        forced_error = g_force_fread_failure_error;
        g_force_fread_failure_enabled = 0;
        g_force_fread_failure_error = ER_SUCCESS;
        if (forced_error == ER_SUCCESS)
            forced_error = FT_ERR_IO;
        ft_errno = forced_error;
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    total_size = size * count;
    total_read = 0;
    byte_buffer = static_cast<char*>(buffer);
    bytes_read = 0;
    while (total_read < total_size)
    {
        bytes_read = su_read(stream->_descriptor,
            byte_buffer + total_read, total_size - total_read);
        if (bytes_read <= 0)
            break;
        total_read += static_cast<size_t>(bytes_read);
    }
    if (bytes_read >= 0)
        ft_errno = ER_SUCCESS;
    su_file_unlock(stream, lock_acquired);
    return (total_read / size);
}

size_t su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t total_size;
    ssize_t bytes_written;
    size_t maximum_size;
    bool lock_acquired;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (size == 0 || count == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (0);
    if (stream->closed == true)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    total_size = size * count;
    bytes_written = su_write(stream->_descriptor, buffer, total_size);
    if (bytes_written < 0)
    {
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    ft_errno = ER_SUCCESS;
    su_file_unlock(stream, lock_acquired);
    return (static_cast<size_t>(bytes_written) / size);
}

int su_fseek(su_file *stream, long offset, int origin)
{
    off_t result;
    bool lock_acquired;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1);
    if (stream->closed == true)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        return (-1);
    }
    result = lseek(stream->_descriptor, offset, origin);
    if (result < 0)
    {
        if (ft_errno != ER_SUCCESS)
            ft_errno = cmp_normalize_ft_errno(ft_errno);
        else if (errno != 0)
            ft_errno = cmp_map_system_error_to_ft(errno);
        su_file_unlock(stream, lock_acquired);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    su_file_unlock(stream, lock_acquired);
    return (0);
}

long su_ftell(su_file *stream)
{
    off_t position;
    bool lock_acquired;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1L);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1L);
    if (stream->closed == true)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        return (-1L);
    }
    position = lseek(stream->_descriptor, 0, SEEK_CUR);
    if (position < 0)
    {
        if (ft_errno != ER_SUCCESS)
            ft_errno = cmp_normalize_ft_errno(ft_errno);
        else if (errno != 0)
            ft_errno = cmp_map_system_error_to_ft(errno);
        su_file_unlock(stream, lock_acquired);
        return (-1L);
    }
    ft_errno = ER_SUCCESS;
    su_file_unlock(stream, lock_acquired);
    return (position);
}

