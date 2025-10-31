#include "gnl_stream.hpp"
#include "../Template/move.hpp"
#include <errno.h>
#ifdef _WIN32
# include <winsock2.h>
#endif

static ssize_t gnl_stream_default_fd_read(void *user_data, char *buffer, size_t max_size) noexcept
{
    int file_descriptor;

    file_descriptor = -1;
    if (user_data)
        file_descriptor = *(static_cast<int *>(user_data));
    if (file_descriptor < 0 || !buffer || max_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
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
            ft_errno = ft_map_system_error(last_error);
        else
            ft_errno = FT_ERR_IO;
#else
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_IO;
#endif
    }
    return (read_result);
}

static ssize_t gnl_stream_default_file_read(void *user_data, char *buffer, size_t max_size) noexcept
{
    FILE *file_handle;

    file_handle = static_cast<FILE *>(user_data);
    if (!file_handle || !buffer || max_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    size_t read_count;

    read_count = fread(buffer, 1, max_size, file_handle);
    if (read_count == 0)
    {
        if (ferror(file_handle))
        {
            ft_errno = FT_ERR_IO;
            return (-1);
        }
    }
    return (static_cast<ssize_t>(read_count));
}

void gnl_stream::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void gnl_stream::set_error(int error_code) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return ;
    }
    this->set_error_unlocked(error_code);
    gnl_stream::restore_errno(guard, entry_errno);
    return ;
}

int gnl_stream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    int entry_errno;

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

void gnl_stream::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != ER_SUCCESS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

gnl_stream::gnl_stream() noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(false)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    this->set_error_unlocked(ER_SUCCESS);
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

    descriptor_copy = file_descriptor;
    return (gnl_stream_default_fd_read(&descriptor_copy, buffer, max_size));
}

ssize_t gnl_stream::read_from_file(FILE *file_handle, char *buffer, size_t max_size) const noexcept
{
    return (gnl_stream_default_file_read(file_handle, buffer, max_size));
}

int gnl_stream::init_from_fd(int file_descriptor) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (lock_error);
    }
    if (file_descriptor < 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        gnl_stream::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->_file_descriptor = file_descriptor;
    this->_file_handle = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(ER_SUCCESS);
    gnl_stream::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int gnl_stream::init_from_file(FILE *file_handle, bool close_on_reset) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (lock_error);
    }
    if (!file_handle)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        gnl_stream::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->_file_handle = file_handle;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = close_on_reset;
    this->set_error_unlocked(ER_SUCCESS);
    gnl_stream::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int gnl_stream::init_from_callback(ssize_t (*callback)(void *user_data, char *buffer, size_t max_size) noexcept,
        void *user_data) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (lock_error);
    }
    if (!callback)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        gnl_stream::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->_read_callback = callback;
    this->_user_data = user_data;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(ER_SUCCESS);
    gnl_stream::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

void gnl_stream::reset() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return ;
    }
    if (this->_close_on_reset && this->_file_handle)
    {
        fclose(this->_file_handle);
    }
    this->_file_handle = ft_nullptr;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    this->set_error_unlocked(ER_SUCCESS);
    gnl_stream::restore_errno(guard, entry_errno);
    return ;
}

ssize_t gnl_stream::read(char *buffer, size_t max_size) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    ssize_t read_result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (-1);
    }
    if (!buffer || max_size == 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        gnl_stream::restore_errno(guard, entry_errno);
        return (-1);
    }
    read_result = -1;
    if (this->_read_callback)
        read_result = this->_read_callback(this->_user_data, buffer, max_size);
    else if (this->_file_descriptor >= 0)
        read_result = this->read_from_descriptor(this->_file_descriptor, buffer, max_size);
    else if (this->_file_handle)
        read_result = this->read_from_file(this->_file_handle, buffer, max_size);
    else
    {
        this->set_error_unlocked(FT_ERR_INVALID_STATE);
        gnl_stream::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (read_result < 0)
    {
        if (ft_errno == ER_SUCCESS)
            this->set_error_unlocked(FT_ERR_IO);
        else
            this->set_error_unlocked(ft_errno);
        gnl_stream::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    gnl_stream::restore_errno(guard, entry_errno);
    return (read_result);
}

int gnl_stream::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_code;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_code = this->_error_code;
    gnl_stream::restore_errno(guard, entry_errno);
    return (error_code);
}

const char *gnl_stream::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}
