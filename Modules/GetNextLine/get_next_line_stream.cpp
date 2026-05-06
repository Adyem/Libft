#include "gnl_stream.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

static int64_t gnl_stream_default_fd_read(void *user_data, char *buffer, ft_size_t max_size,
    int32_t *error_code)
{
    int32_t file_descriptor;
    int64_t read_result;
    int32_t read_error;

    file_descriptor = -1;
    read_result = -1;
    read_error = FT_ERR_SUCCESS;
    if (error_code)
        *error_code = FT_ERR_SUCCESS;
    if (user_data)
        file_descriptor = *(static_cast<int32_t *>(user_data));
    if (file_descriptor < 0 || !buffer || max_size == 0)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    read_error = cmp_read(file_descriptor, buffer, max_size, &read_result);
    if (read_error != FT_ERR_SUCCESS)
    {
        if (error_code)
            *error_code = read_error;
        return (-1);
    }
    return (read_result);
}

static int64_t gnl_stream_default_file_read(void *user_data, char *buffer, ft_size_t max_size,
    int32_t *error_code) noexcept
{
    FILE *file_handle;
    ft_size_t read_count;

    file_handle = static_cast<FILE *>(user_data);
    if (error_code)
        *error_code = FT_ERR_SUCCESS;
    if (!file_handle || !buffer || max_size == 0)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
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
    return (static_cast<int64_t>(read_count));
}

gnl_stream::gnl_stream() noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(FT_FALSE)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _mutex(ft_nullptr)
{
    return ;
}

void gnl_stream::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

gnl_stream::gnl_stream(const gnl_stream &other) noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(FT_FALSE)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _mutex(ft_nullptr)
{
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("gnl_stream::gnl_stream(copy)",
            "called with uninitialised source object");
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_read_callback = other._read_callback;
    this->_user_data = other._user_data;
    this->_file_descriptor = other._file_descriptor;
    this->_file_handle = other._file_handle;
    this->_close_on_reset = other._close_on_reset;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return ;
}

gnl_stream::gnl_stream(gnl_stream &&other) noexcept
    : _read_callback(ft_nullptr)
    , _user_data(ft_nullptr)
    , _file_descriptor(-1)
    , _file_handle(ft_nullptr)
    , _close_on_reset(FT_FALSE)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _mutex(ft_nullptr)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("gnl_stream::gnl_stream(move)",
            "called with uninitialised source object");
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->move(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

gnl_stream::~gnl_stream() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t gnl_stream::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "gnl_stream::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_read_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

uint32_t gnl_stream::move(gnl_stream &other) noexcept
{
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("gnl_stream::move",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->_read_callback = ft_nullptr;
            this->_user_data = ft_nullptr;
            this->_file_descriptor = -1;
            this->_file_handle = ft_nullptr;
            this->_close_on_reset = FT_FALSE;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (static_cast<uint32_t>(destroy_error));
        }
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_read_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_file_descriptor = -1;
        this->_file_handle = ft_nullptr;
        this->_close_on_reset = FT_FALSE;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        if (this->initialize() != FT_ERR_SUCCESS)
        {
            this->_read_callback = ft_nullptr;
            this->_user_data = ft_nullptr;
            this->_file_descriptor = -1;
            this->_file_handle = ft_nullptr;
            this->_close_on_reset = FT_FALSE;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (FT_ERR_INITIALIZATION_FAILED);
        }
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_read_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_file_descriptor = -1;
        this->_file_handle = ft_nullptr;
        this->_close_on_reset = FT_FALSE;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (static_cast<uint32_t>(lock_error));
    }
    this->_read_callback = other._read_callback;
    this->_user_data = other._user_data;
    this->_file_descriptor = other._file_descriptor;
    this->_file_handle = other._file_handle;
    this->_close_on_reset = other._close_on_reset;
    other._read_callback = ft_nullptr;
    other._user_data = ft_nullptr;
    other._file_descriptor = -1;
    other._file_handle = ft_nullptr;
    other._close_on_reset = FT_FALSE;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (FT_ERR_SUCCESS);
}

int32_t gnl_stream::destroy() noexcept
{
    int32_t first_error;
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    if (this->is_thread_safe() == FT_TRUE)
    {
        disable_error = this->disable_thread_safety();
        if (disable_error != FT_ERR_SUCCESS)
            first_error = disable_error;
    }
    this->reset();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (first_error != FT_ERR_SUCCESS)
        return (first_error);
    return (FT_ERR_SUCCESS);
}

int32_t gnl_stream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::enable_thread_safety");
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

int32_t gnl_stream::disable_thread_safety(void) noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool gnl_stream::is_thread_safe(void) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int64_t gnl_stream::read_from_descriptor(int32_t file_descriptor, char *buffer, ft_size_t max_size, int32_t *error_code) const noexcept
{
    int32_t descriptor_copy;

    descriptor_copy = file_descriptor;
    return (gnl_stream_default_fd_read(&descriptor_copy, buffer, max_size, error_code));
}

int64_t gnl_stream::read_from_file(FILE *file_handle, char *buffer, ft_size_t max_size, int32_t *error_code) const noexcept
{
    return (gnl_stream_default_file_read(file_handle, buffer, max_size, error_code));
}

int32_t gnl_stream::init_from_fd(int32_t file_descriptor) noexcept
{
    int32_t lock_error;
    int32_t operation_error;

    operation_error = FT_ERR_SUCCESS;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::init_from_fd");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (file_descriptor < 0)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (operation_error);
    }
    this->_file_descriptor = file_descriptor;
    this->_file_handle = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = FT_FALSE;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (operation_error);
}

int32_t gnl_stream::init_from_file(FILE *file_handle, ft_bool close_on_reset) noexcept
{
    int32_t lock_error;
    int32_t operation_error;

    operation_error = FT_ERR_SUCCESS;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::init_from_file");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (!file_handle)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (operation_error);
    }
    this->_file_handle = file_handle;
    this->_file_descriptor = -1;
    this->_user_data = ft_nullptr;
    this->_read_callback = ft_nullptr;
    this->_close_on_reset = close_on_reset;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (operation_error);
}

int32_t gnl_stream::init_from_callback(int64_t (*callback)(void *user_data, char *buffer, ft_size_t max_size) noexcept,
        void *user_data) noexcept
{
    int32_t lock_error;
    int32_t operation_error;

    operation_error = FT_ERR_SUCCESS;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "gnl_stream::init_from_callback");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (!callback)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (operation_error);
    }
    this->_read_callback = callback;
    this->_user_data = user_data;
    this->_file_descriptor = -1;
    this->_file_handle = ft_nullptr;
    this->_close_on_reset = FT_FALSE;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (operation_error);
}

void gnl_stream::reset() noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "gnl_stream::reset");
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
    this->_close_on_reset = FT_FALSE;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int64_t gnl_stream::read(char *buffer, ft_size_t max_size) noexcept
{
    int32_t lock_error;
    int32_t operation_error;
    int64_t read_result;

    operation_error = FT_ERR_SUCCESS;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "gnl_stream::read");
    lock_error = FT_ERR_SUCCESS;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (!buffer || max_size == 0)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (-1);
    }
    if (read_result < 0)
    {
        if (operation_error == FT_ERR_SUCCESS || operation_error == FT_ERR_INVALID_HANDLE)
            operation_error = FT_ERR_IO;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (-1);
    }
    operation_error = FT_ERR_SUCCESS;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (read_result);
}
