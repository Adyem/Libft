#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <cstdlib>
#include <new>
#include <limits>

static bool g_force_file_stream_allocation_failure = false;

int su_file_prepare_thread_safety(su_file *stream)
{
    pt_mutex    *mutex_pointer;
    int         mutex_error;

    if (stream == ft_nullptr)
        return (-1);
    if (stream->mutex != ft_nullptr)
        return (0);
    mutex_pointer = new (std::nothrow) pt_mutex;
    if (mutex_pointer == ft_nullptr)
        return (-1);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    stream->mutex = mutex_pointer;
    return (0);
}

void su_file_teardown_thread_safety(su_file *stream)
{
    if (stream == ft_nullptr)
        return ;
    if (stream->mutex == ft_nullptr)
        return ;
    (void)stream->mutex->destroy();
    delete stream->mutex;
    stream->mutex = ft_nullptr;
    return ;
}

void su_force_file_stream_allocation_failure(bool should_fail)
{
    g_force_file_stream_allocation_failure = should_fail;
    return ;
}

int su_file_lock(su_file *stream, bool *lock_acquired)
{
    int lock_error;

    if (stream == ft_nullptr || lock_acquired == ft_nullptr)
        return (-1);
    *lock_acquired = false;
    if (stream->mutex == ft_nullptr)
        return (0);
    lock_error = stream->mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    *lock_acquired = true;
    return (0);
}

void su_file_unlock(su_file *stream, bool lock_acquired)
{
    if (stream == ft_nullptr)
        return ;
    if (lock_acquired == false)
        return ;
    if (stream->mutex == ft_nullptr)
        return ;
    (void)stream->mutex->unlock();
    return ;
}

static su_file *create_file_stream(int file_descriptor)
{
    su_file  *file_stream;

    if (file_descriptor < 0)
        return (ft_nullptr);
    if (g_force_file_stream_allocation_failure == true)
    {
        (void)cmp_close(file_descriptor);
        return (ft_nullptr);
    }
    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
    {
        (void)cmp_close(file_descriptor);
        return (ft_nullptr);
    }
    file_stream->mutex = ft_nullptr;
    file_stream->closed = false;
    file_stream->_descriptor = file_descriptor;
    if (su_file_prepare_thread_safety(file_stream) != 0)
    {
        (void)cmp_close(file_descriptor);
        std::free(file_stream);
        return (ft_nullptr);
    }
    return (file_stream);
}

su_file *su_fopen(const char *path_name)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
        return (ft_nullptr);
    file_descriptor = su_open(path_name);
    if (file_descriptor < 0)
        return (ft_nullptr);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
        return (ft_nullptr);
    file_descriptor = su_open(path_name, flags);
    if (file_descriptor < 0)
        return (ft_nullptr);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags, mode_t mode)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
        return (ft_nullptr);
    file_descriptor = su_open(path_name, flags, mode);
    if (file_descriptor < 0)
        return (ft_nullptr);
    return (create_file_stream(file_descriptor));
}

int su_fclose(su_file *stream)
{
    bool    lock_acquired;
    int     close_result;

    if (stream == ft_nullptr)
        return (-1);
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1);
    if (stream->closed == true)
    {
        su_file_unlock(stream, lock_acquired);
        return (-1);
    }
    stream->closed = true;
    close_result = cmp_close(stream->_descriptor);
    if (close_result != 0)
    {
        stream->closed = false;
        su_file_unlock(stream, lock_acquired);
        return (-1);
    }
    su_file_unlock(stream, lock_acquired);
    su_file_teardown_thread_safety(stream);
    std::free(stream);
    return (0);
}

size_t su_fread(void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t      total_size;
    size_t      total_read;
    char        *byte_buffer;
    ssize_t     bytes_read;
    size_t      maximum_size;
    bool        lock_acquired;

    if (buffer == ft_nullptr || stream == ft_nullptr)
        return (0);
    if (size == 0 || count == 0)
        return (0);
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (0);
    if (stream->closed == true)
    {
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    total_size = size * count;
    total_read = 0;
    byte_buffer = static_cast<char*>(buffer);
    while (total_read < total_size)
    {
        bytes_read = su_read(stream->_descriptor,
            byte_buffer + total_read, total_size - total_read);
        if (bytes_read <= 0)
            break;
        total_read += static_cast<size_t>(bytes_read);
    }
    su_file_unlock(stream, lock_acquired);
    return (total_read / size);
}

size_t su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t      total_size;
    size_t      maximum_size;
    bool        lock_acquired;
    ssize_t     bytes_written;

    if (buffer == ft_nullptr || stream == ft_nullptr)
        return (0);
    if (size == 0 || count == 0)
        return (0);
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (0);
    if (stream->closed == true)
    {
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    maximum_size = std::numeric_limits<size_t>::max();
    if (count > maximum_size / size)
    {
        su_file_unlock(stream, lock_acquired);
        return (0);
    }
    total_size = size * count;
    bytes_written = su_write(stream->_descriptor, buffer, total_size);
    su_file_unlock(stream, lock_acquired);
    if (bytes_written < 0)
        return (0);
    return (static_cast<size_t>(bytes_written) / size);
}

int su_fseek(su_file *stream, long offset, int origin)
{
    off_t   result;
    bool    lock_acquired;

    if (stream == ft_nullptr)
        return (-1);
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1);
    if (stream->closed == true)
    {
        su_file_unlock(stream, lock_acquired);
        return (-1);
    }
    result = lseek(stream->_descriptor, offset, origin);
    su_file_unlock(stream, lock_acquired);
    if (result < 0)
        return (-1);
    return (0);
}

long su_ftell(su_file *stream)
{
    off_t   position;
    bool    lock_acquired;

    if (stream == ft_nullptr)
        return (-1L);
    if (su_file_lock(stream, &lock_acquired) != 0)
        return (-1L);
    if (stream->closed == true)
    {
        su_file_unlock(stream, lock_acquired);
        return (-1L);
    }
    position = lseek(stream->_descriptor, 0, SEEK_CUR);
    su_file_unlock(stream, lock_acquired);
    if (position < 0)
        return (-1L);
    return (position);
}
