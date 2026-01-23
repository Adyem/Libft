#include "gnl_stream.hpp"
#include "../Template/move.hpp"
#include <errno.h>
#ifdef _WIN32
# include <winsock2.h>
#endif

static ssize_t gnl_stream_default_fd_read(void *user_data, char *buffer, size_t max_size,
    int *error_code)
{
    int file_descriptor;

    file_descriptor = -1;
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    if (user_data)
        file_descriptor = *(static_cast<int *>(user_data));
    if (file_descriptor < 0 || !buffer || max_size == 0)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ssize_t read_result;

    read_result = read(file_descriptor, buffer, max_size);
    if (read_result < 0)
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
        {
            if (error_code)
                *error_code = ft_map_system_error(last_error);
        }
        else
        {
            if (error_code)
                *error_code = FT_ERR_IO;
        }
#else
        if (errno != 0)
        {
            if (error_code)
                *error_code = ft_map_system_error(errno);
        }
        else
        {
            if (error_code)
                *error_code = FT_ERR_IO;
        }
#endif
    }
    return (read_result);
}

static ssize_t gnl_stream_default_file_read(void *user_data, char *buffer, size_t max_size,
    int *error_code) noexcept
{
    FILE *file_handle;

    file_handle = static_cast<FILE *>(user_data);
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    if (!file_handle || !buffer || max_size == 0)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    size_t read_count;

    read_count = fread(buffer, 1, max_size, file_handle);
    if (read_count == 0)
    {
        if (ferror(file_handle))
        {
            if (error_code)
                *error_code = FT_ERR_IO;
            return (-1);
        }
    }
    return (static_cast<ssize_t>(read_count));
}

void gnl_stream::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

void gnl_stream::set_error(int error_code) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    this->set_error_unlocked(error_code);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return ;
}

int gnl_stream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.last_operation_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.last_operation_error());
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

gnl_stream::gnl_stream() noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(false)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

gnl_stream::~gnl_stream() noexcept
{
    this->reset();
    return ;
}

ssize_t gnl_stream::read_from_descriptor(int file_descriptor, char *buffer, size_t max_size) const noexcept
{
    int descriptor_copy;
    int error_code;

    descriptor_copy = file_descriptor;
    error_code = FT_ERR_SUCCESSS;
    ssize_t read_result = gnl_stream_default_fd_read(&descriptor_copy, buffer, max_size, &error_code);
    if (read_result < 0)
        this->set_error_unlocked(error_code);
    return (read_result);
}

ssize_t gnl_stream::read_from_file(FILE *file_handle, char *buffer, size_t max_size) const noexcept
{
    int error_code;
    ssize_t read_result;

    error_code = FT_ERR_SUCCESSS;
    read_result = gnl_stream_default_file_read(file_handle, buffer, max_size, &error_code);
    if (read_result < 0)
        this->set_error_unlocked(error_code);
    return (read_result);
}

int gnl_stream::init_from_fd(int file_descriptor) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (file_descriptor < 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (this->_error_code);
    }
    this->_file_descriptor = file_descriptor;
    this->_file_handle = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return (FT_ERR_SUCCESSS);
}

int gnl_stream::init_from_file(FILE *file_handle, bool close_on_reset) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (!file_handle)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (this->_error_code);
    }
    this->_file_handle = file_handle;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = close_on_reset;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return (FT_ERR_SUCCESSS);
}

int gnl_stream::init_from_callback(ssize_t (*callback)(void *user_data, char *buffer, size_t max_size) noexcept,
        void *user_data) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (!callback)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (this->_error_code);
    }
    this->_read_callback = callback;
    this->_user_data = user_data;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return (FT_ERR_SUCCESSS);
}

void gnl_stream::reset() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    if (this->_close_on_reset && this->_file_handle)
    {
        fclose(this->_file_handle);
    }
    this->_file_handle = ft_nullptr;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return ;
}

ssize_t gnl_stream::read(char *buffer, size_t max_size) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    ssize_t read_result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (-1);
    if (!buffer || max_size == 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (-1);
    }
    read_result = -1;
    if (this->_read_callback)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        read_result = this->_read_callback(this->_user_data, buffer, max_size);
        if (read_result < 0)
        {
            int callback_error;

            callback_error = ft_global_error_stack_pop_newest();
            if (callback_error == FT_ERR_SUCCESSS)
                callback_error = FT_ERR_IO;
            this->set_error_unlocked(callback_error);
        }
    }
    else if (this->_file_descriptor >= 0)
        read_result = this->read_from_descriptor(this->_file_descriptor, buffer, max_size);
    else if (this->_file_handle)
        read_result = this->read_from_file(this->_file_handle, buffer, max_size);
    else
    {
        this->set_error_unlocked(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (-1);
    }
    if (read_result < 0)
    {
        int read_error;

        read_error = this->_error_code;
        if (read_error == FT_ERR_SUCCESSS)
            read_error = FT_ERR_IO;
        else if (read_error == FT_ERR_INVALID_HANDLE)
            read_error = FT_ERR_IO;
        this->set_error_unlocked(read_error);
        if (guard.owns_lock())
            guard.unlock();
        if (guard.last_operation_error() != FT_ERR_SUCCESSS)
            this->set_error_unlocked(guard.last_operation_error());
        return (-1);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(guard.last_operation_error());
    return (read_result);
}

int gnl_stream::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_code;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    error_code = this->_error_code;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.last_operation_error() != FT_ERR_SUCCESSS)
        return (guard.last_operation_error());
    return (error_code);
}

const char *gnl_stream::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}
