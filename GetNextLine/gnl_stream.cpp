#include "gnl_stream.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <errno.h>
#include <new>
#ifdef _WIN32
# include <winsock2.h>
#endif

static ssize_t gnl_stream_default_fd_read(void *user_data, char *buffer, size_t max_size,
    int *error_code)
{
    int file_descriptor;

    file_descriptor = -1;
    if (error_code)
        *error_code = FT_ERR_SUCCESS;
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
                *error_code = cmp_map_system_error_to_ft(last_error);
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
                *error_code = cmp_map_system_error_to_ft(errno);
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
        *error_code = FT_ERR_SUCCESS;
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

gnl_stream::gnl_stream() noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(false)
    , _initialized_state(gnl_stream::_state_uninitialized)
    , _mutex(ft_nullptr)
{
    return ;
}

gnl_stream::~gnl_stream() noexcept
{
    if (this->_initialized_state == gnl_stream::_state_initialized)
        (void)this->destroy();
    if (this->_mutex != ft_nullptr)
    {
        (void)this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

void gnl_stream::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "gnl_stream lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void gnl_stream::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == gnl_stream::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int gnl_stream::initialize() noexcept
{
    if (this->_initialized_state == gnl_stream::_state_initialized)
    {
        this->abort_lifecycle_error("gnl_stream::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_read_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = false;
    this->_initialized_state = gnl_stream::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int gnl_stream::destroy() noexcept
{
    if (this->_initialized_state != gnl_stream::_state_initialized)
    {
        return (FT_ERR_INVALID_STATE);
    }
    this->reset();
    this->disable_thread_safety();
    this->_initialized_state = gnl_stream::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int gnl_stream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("gnl_stream::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int gnl_stream::disable_thread_safety(void) noexcept
{
    int destroy_error;

    this->abort_if_not_initialized("gnl_stream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool gnl_stream::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("gnl_stream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ssize_t gnl_stream::read_from_descriptor(int file_descriptor, char *buffer, size_t max_size, int *error_code) const noexcept
{
    int descriptor_copy;

    descriptor_copy = file_descriptor;
    return (gnl_stream_default_fd_read(&descriptor_copy, buffer, max_size, error_code));
}

ssize_t gnl_stream::read_from_file(FILE *file_handle, char *buffer, size_t max_size, int *error_code) const noexcept
{
    return (gnl_stream_default_file_read(file_handle, buffer, max_size, error_code));
}

int gnl_stream::init_from_fd(int file_descriptor) noexcept
{
    int lock_error;
    int unlock_error;
    int operation_error;

    operation_error = FT_ERR_SUCCESS;
    this->abort_if_not_initialized("gnl_stream::init_from_fd");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (file_descriptor < 0)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (operation_error);
    }
    this->_file_descriptor = file_descriptor;
    this->_file_handle = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    unlock_error = FT_ERR_SUCCESS;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

int gnl_stream::init_from_file(FILE *file_handle, bool close_on_reset) noexcept
{
    int lock_error;
    int unlock_error;
    int operation_error;

    operation_error = FT_ERR_SUCCESS;
    this->abort_if_not_initialized("gnl_stream::init_from_file");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (!file_handle)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (operation_error);
    }
    this->_file_handle = file_handle;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = close_on_reset;
    unlock_error = FT_ERR_SUCCESS;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

int gnl_stream::init_from_callback(ssize_t (*callback)(void *user_data, char *buffer, size_t max_size) noexcept,
        void *user_data) noexcept
{
    int lock_error;
    int unlock_error;
    int operation_error;

    operation_error = FT_ERR_SUCCESS;
    this->abort_if_not_initialized("gnl_stream::init_from_callback");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (!callback)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (operation_error);
    }
    this->_read_callback = callback;
    this->_user_data = user_data;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = false;
    unlock_error = FT_ERR_SUCCESS;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

void gnl_stream::reset() noexcept
{
    int lock_error;

    this->abort_if_not_initialized("gnl_stream::reset");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_close_on_reset && this->_file_handle)
        fclose(this->_file_handle);
    this->_file_handle = ft_nullptr;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = false;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

ssize_t gnl_stream::read(char *buffer, size_t max_size) noexcept
{
    int lock_error;
    int unlock_error;
    int operation_error;
    ssize_t read_result;

    operation_error = FT_ERR_SUCCESS;
    this->abort_if_not_initialized("gnl_stream::read");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (!buffer || max_size == 0)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (-1);
    }
    read_result = -1;
    if (this->_read_callback)
    {
        read_result = this->_read_callback(this->_user_data, buffer, max_size);
        if (read_result < 0)
            operation_error = FT_ERR_IO;
    }
    else if (this->_file_descriptor >= 0)
        read_result = this->read_from_descriptor(this->_file_descriptor, buffer, max_size, &operation_error);
    else if (this->_file_handle)
        read_result = this->read_from_file(this->_file_handle, buffer, max_size, &operation_error);
    else
    {
        operation_error = FT_ERR_INVALID_STATE;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (-1);
    }
    if (read_result < 0)
    {
        if (operation_error == FT_ERR_SUCCESS || operation_error == FT_ERR_INVALID_HANDLE)
            operation_error = FT_ERR_IO;
        unlock_error = FT_ERR_SUCCESS;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            operation_error = unlock_error;
        return (-1);
    }
    operation_error = FT_ERR_SUCCESS;
    unlock_error = FT_ERR_SUCCESS;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (read_result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *gnl_stream::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
