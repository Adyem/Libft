#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstdlib>
#include <cerrno>
#include <limits>

static bool g_force_file_stream_allocation_failure = false;

int su_file_prepare_thread_safety(su_file *stream)
{
    if (stream == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (stream->mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    pt_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(mutex_error);
        return (-1);
    }
    stream->mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

void su_file_teardown_thread_safety(su_file *stream)
{
    if (stream == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    pt_mutex_destroy(&stream->mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

void su_force_file_stream_allocation_failure(bool should_fail)
{
    g_force_file_stream_allocation_failure = should_fail;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

static su_file *create_file_stream(int file_descriptor, int *error_code)
{
    su_file *file_stream;
    bool    prepared;
    int     local_error_code;

    if (file_descriptor < 0)
    {
        if (error_code != ft_nullptr && *error_code == FT_ERR_SUCCESS)
            *error_code = FT_ERR_INVALID_HANDLE;
        return (ft_nullptr);
    }
    if (g_force_file_stream_allocation_failure == true)
    {
        cmp_close(file_descriptor);
        if (error_code != ft_nullptr && *error_code == FT_ERR_SUCCESS)
            *error_code = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
    {
        cmp_close(file_descriptor);
        if (error_code != ft_nullptr && *error_code == FT_ERR_SUCCESS)
            *error_code = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    file_stream->mutex = ft_nullptr;
    file_stream->closed = false;
    file_stream->_descriptor = file_descriptor;
    prepared = (su_file_prepare_thread_safety(file_stream) == 0);
    local_error_code = ft_global_error_stack_drop_last_error();
    if (prepared == false)
    {
        file_stream->_descriptor = -1;
        std::free(file_stream);
        cmp_close(file_descriptor);
        if (error_code != ft_nullptr && *error_code == FT_ERR_SUCCESS)
        {
            if (local_error_code == FT_ERR_SUCCESS)
                local_error_code = FT_ERR_INTERNAL;
            *error_code = local_error_code;
        }
        return (ft_nullptr);
    }
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESS;
    return (file_stream);
}

su_file *su_fopen(const char *path_name)
{
    int file_descriptor;
    int error_code;
    su_file *stream;

    if (path_name == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name);
    error_code = ft_global_error_stack_drop_last_error();
    stream = create_file_stream(file_descriptor, &error_code);
    if (stream == ft_nullptr)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    ft_global_error_stack_push(error_code);
    return (stream);
}

su_file *su_fopen(const char *path_name, int flags)
{
    int file_descriptor;
    int error_code;
    su_file *stream;

    if (path_name == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags);
    error_code = ft_global_error_stack_drop_last_error();
    stream = create_file_stream(file_descriptor, &error_code);
    if (stream == ft_nullptr)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    ft_global_error_stack_push(error_code);
    return (stream);
}

su_file *su_fopen(const char *path_name, int flags, mode_t mode)
{
    int file_descriptor;
    int error_code;
    su_file *stream;

    if (path_name == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags, mode);
    error_code = ft_global_error_stack_drop_last_error();
    stream = create_file_stream(file_descriptor, &error_code);
    return (stream);
}

int su_fclose(su_file *stream)
{
    int result;
    bool lock_acquired;
    int error_code;

    if (stream == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    lock_acquired = false;
    if (stream->mutex != ft_nullptr)
    {
        stream->mutex->lock(THREAD_ID);
        int lock_error = ft_global_error_stack_drop_last_error();
        if (lock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(lock_error);
            return (-1);
        }
        lock_acquired = true;
    }
    stream->closed = true;
    result = cmp_close(stream->_descriptor);
    if (result == 0)
    {
        if (lock_acquired)
        {
            stream->mutex->unlock(THREAD_ID);
            ft_global_error_stack_drop_last_error();
        }
        su_file_teardown_thread_safety(stream);
        ft_global_error_stack_drop_last_error();
        std::free(stream);
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    stream->closed = false;
    if (lock_acquired)
    {
        stream->mutex->unlock(THREAD_ID);
        ft_global_error_stack_drop_last_error();
    }
    error_code = FT_ERR_IO;
    if (errno != 0)
        error_code = ft_map_system_error(errno);
    if (error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_IO;
    ft_global_error_stack_push(error_code);
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
    int error_code;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (size == 0 || count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    lock_acquired = false;
    if (stream->mutex != ft_nullptr)
    {
        int lock_error = stream->mutex->lock(THREAD_ID);
        if (lock_error != FT_ERR_SUCCESS)
            return (0);
        ft_global_error_stack_drop_last_error();
        lock_acquired = true;
    }
    if (stream->closed == true)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        if (lock_acquired)
        {
            stream->mutex->unlock(THREAD_ID);
            ft_global_error_stack_drop_last_error();
        }
        ft_global_error_stack_push(error_code);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        if (lock_acquired)
        {
            stream->mutex->unlock(THREAD_ID);
            ft_global_error_stack_drop_last_error();
        }
        ft_global_error_stack_push(error_code);
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
        error_code = ft_global_error_stack_drop_last_error();
        if (bytes_read <= 0)
            break;
        total_read += static_cast<size_t>(bytes_read);
    }
    if (bytes_read >= 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_SUCCESS;
    else if (bytes_read < 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_IO;
    if (lock_acquired)
    {
        stream->mutex->unlock(THREAD_ID);
        ft_global_error_stack_drop_last_error();
    }
    ft_global_error_stack_push(error_code);
    return (total_read / size);
}

size_t su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t total_size;
    ssize_t bytes_written;
    size_t maximum_size;
    bool lock_acquired;
    int error_code;
    int lock_error;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (size == 0 || count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
    {
        lock_error = ft_global_error_stack_drop_last_error();
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    ft_global_error_stack_drop_last_error();
    if (stream->closed == true)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    total_size = size * count;
    bytes_written = su_write(stream->_descriptor, buffer, total_size);
    error_code = ft_global_error_stack_drop_last_error();
    if (bytes_written < 0)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    su_file_unlock(stream, lock_acquired);
    ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (static_cast<size_t>(bytes_written) / size);
}

int su_fseek(su_file *stream, long offset, int origin)
{
    off_t result;
    bool lock_acquired;
    int error_code;
    int lock_error;

    if (stream == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
    {
        lock_error = ft_global_error_stack_drop_last_error();
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    if (stream->closed == true)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    result = lseek(stream->_descriptor, offset, origin);
    if (result < 0)
    {
        error_code = FT_ERR_IO;
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    su_file_unlock(stream, lock_acquired);
    ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

long su_ftell(su_file *stream)
{
    off_t position;
    bool lock_acquired;
    int error_code;
    int lock_error;

    if (stream == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1L);
    }
    lock_acquired = false;
    if (su_file_lock(stream, &lock_acquired) != 0)
    {
        lock_error = ft_global_error_stack_drop_last_error();
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        ft_global_error_stack_push(lock_error);
        return (-1L);
    }
    ft_global_error_stack_drop_last_error();
    if (stream->closed == true)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (-1L);
    }
    position = lseek(stream->_descriptor, 0, SEEK_CUR);
    if (position < 0)
    {
        error_code = FT_ERR_IO;
        if (errno != 0)
            error_code = ft_map_system_error(errno);
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        su_file_unlock(stream, lock_acquired);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (-1L);
    }
    su_file_unlock(stream, lock_acquired);
    ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (position);
}
