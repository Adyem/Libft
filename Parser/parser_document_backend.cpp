#include "document_backend.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CMA/CMA.hpp"
#include "../Networking/http_client.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include <fcntl.h>
#include <new>

ft_document_source::ft_document_source() noexcept
    : _mutex(ft_nullptr), _initialized_state(ft_document_source::_state_uninitialized)
{
    return ;
}

ft_document_source::~ft_document_source()
{
    if (this->_initialized_state == ft_document_source::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_document_source::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_document_source lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_document_source::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_document_source::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_document_source::initialize() noexcept
{
    if (this->_initialized_state == ft_document_source::_state_initialized)
    {
        this->abort_lifecycle_error("ft_document_source::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_document_source::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_document_source::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_document_source::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_document_source::_state_destroyed;
    return (disable_error);
}

int ft_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_document_source::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_document_source::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_document_source::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_document_source::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_document_source::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_document_source::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_document_source::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_document_source::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_document_sink::ft_document_sink() noexcept
    : _mutex(ft_nullptr), _initialized_state(ft_document_sink::_state_uninitialized)
{
    return ;
}

ft_document_sink::~ft_document_sink()
{
    if (this->_initialized_state == ft_document_sink::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_document_sink::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_document_sink lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_document_sink::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_document_sink::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_document_sink::initialize() noexcept
{
    if (this->_initialized_state == ft_document_sink::_state_initialized)
    {
        this->abort_lifecycle_error("ft_document_sink::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_document_sink::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_document_sink::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_document_sink::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_document_sink::_state_destroyed;
    return (disable_error);
}

int ft_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_document_sink::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_document_sink::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_document_sink::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_document_sink::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_document_sink::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_document_sink::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_document_sink::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_document_sink::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_file_document_source::ft_file_document_source(const char *file_path) noexcept
    : _path(), _mutex(ft_nullptr),
      _initialized_state(ft_file_document_source::_state_uninitialized)
{
    if (file_path != ft_nullptr)
        this->_path = file_path;
    return ;
}

ft_file_document_source::~ft_file_document_source()
{
    if (this->_initialized_state == ft_file_document_source::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_file_document_source::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_file_document_source lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_file_document_source::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_file_document_source::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_file_document_source::initialize() noexcept
{
    if (this->_initialized_state == ft_file_document_source::_state_initialized)
    {
        this->abort_lifecycle_error("ft_file_document_source::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_file_document_source::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_source::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_file_document_source::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_path.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_file_document_source::_state_destroyed;
    return (disable_error);
}

int ft_file_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_file_document_source::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_source::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_file_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_file_document_source::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_source::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_file_document_source::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_file_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_file_document_source::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_file_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_file_document_source::set_path(const char *file_path) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_file_document_source::set_path");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (file_path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = file_path;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const char *ft_file_document_source::get_path() const noexcept
{
    this->abort_if_not_initialized("ft_file_document_source::get_path");
    return (this->_path.c_str());
}

int ft_file_document_source::read_all(ft_string &output)
{
    su_file *file_stream;
    long file_size_long;
    size_t file_size;
    char *content_pointer;
    size_t read_count;
    int error_code;

    this->abort_if_not_initialized("ft_file_document_source::read_all");
    file_stream = su_fopen(this->_path.c_str());
    if (file_stream == ft_nullptr)
        return (FT_ERR_IO);
    file_size_long = 0;
    file_size = 0;
    content_pointer = ft_nullptr;
    read_count = 0;
    error_code = FT_ERR_SUCCESS;
    if (su_fseek(file_stream, 0, SEEK_END) != 0)
        error_code = FT_ERR_IO;
    if (error_code == FT_ERR_SUCCESS)
    {
        file_size_long = su_ftell(file_stream);
        if (file_size_long < 0)
            error_code = FT_ERR_IO;
    }
    if (error_code == FT_ERR_SUCCESS && su_fseek(file_stream, 0, SEEK_SET) != 0)
        error_code = FT_ERR_IO;
    if (error_code == FT_ERR_SUCCESS)
    {
        if (static_cast<unsigned long long>(file_size_long)
            > static_cast<unsigned long long>(static_cast<size_t>(-1)))
            error_code = FT_ERR_OUT_OF_RANGE;
    }
    if (error_code == FT_ERR_SUCCESS)
    {
        file_size = static_cast<size_t>(file_size_long);
        content_pointer = static_cast<char *>(cma_malloc(file_size + 1));
        if (content_pointer == ft_nullptr)
            error_code = FT_ERR_NO_MEMORY;
    }
    if (error_code == FT_ERR_SUCCESS)
    {
        read_count = su_fread(content_pointer, 1, file_size, file_stream);
        if (read_count != file_size)
            error_code = FT_ERR_IO;
    }
    if (error_code == FT_ERR_SUCCESS)
        content_pointer[file_size] = '\0';
    if (su_fclose(file_stream) != 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_IO;
    if (error_code != FT_ERR_SUCCESS)
    {
        if (content_pointer != ft_nullptr)
            cma_free(content_pointer);
        return (error_code);
    }
    output.assign(content_pointer, file_size);
    cma_free(content_pointer);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_file_document_source::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_file_document_source::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_file_document_sink::ft_file_document_sink(const char *file_path) noexcept
    : _path(), _mutex(ft_nullptr),
      _initialized_state(ft_file_document_sink::_state_uninitialized)
{
    if (file_path != ft_nullptr)
        this->_path = file_path;
    return ;
}

ft_file_document_sink::~ft_file_document_sink()
{
    if (this->_initialized_state == ft_file_document_sink::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_file_document_sink::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_file_document_sink lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_file_document_sink::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_file_document_sink::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_file_document_sink::initialize() noexcept
{
    if (this->_initialized_state == ft_file_document_sink::_state_initialized)
    {
        this->abort_lifecycle_error("ft_file_document_sink::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_file_document_sink::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_sink::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_file_document_sink::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_path.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_file_document_sink::_state_destroyed;
    return (disable_error);
}

int ft_file_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_file_document_sink::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_sink::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_file_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_file_document_sink::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_file_document_sink::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_file_document_sink::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_file_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_file_document_sink::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_file_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_file_document_sink::set_path(const char *file_path) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_file_document_sink::set_path");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (file_path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = file_path;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const char *ft_file_document_sink::get_path() const noexcept
{
    this->abort_if_not_initialized("ft_file_document_sink::get_path");
    return (this->_path.c_str());
}

int ft_file_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    su_file *file_stream;
    size_t written_count;
    int error_code;

    this->abort_if_not_initialized("ft_file_document_sink::write_all");
    if (data_pointer == ft_nullptr && data_length != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    file_stream = su_fopen(this->_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_stream == ft_nullptr)
        return (FT_ERR_IO);
    written_count = 0;
    error_code = FT_ERR_SUCCESS;
    if (data_length > 0)
    {
        written_count = su_fwrite(data_pointer, 1, data_length, file_stream);
        if (written_count != data_length)
            error_code = FT_ERR_IO;
    }
    if (su_fclose(file_stream) != 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_IO;
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_file_document_sink::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_file_document_sink::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
ft_memory_document_source::ft_memory_document_source() noexcept
    : _data_pointer(ft_nullptr), _data_length(0), _mutex(ft_nullptr),
      _initialized_state(ft_memory_document_source::_state_uninitialized)
{
    return ;
}

ft_memory_document_source::ft_memory_document_source(const char *data_pointer,
    size_t data_length) noexcept
    : _data_pointer(data_pointer), _data_length(data_length), _mutex(ft_nullptr),
      _initialized_state(ft_memory_document_source::_state_uninitialized)
{
    return ;
}

ft_memory_document_source::~ft_memory_document_source()
{
    if (this->_initialized_state == ft_memory_document_source::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_memory_document_source::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_memory_document_source lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_memory_document_source::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_memory_document_source::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_memory_document_source::initialize() noexcept
{
    if (this->_initialized_state == ft_memory_document_source::_state_initialized)
    {
        this->abort_lifecycle_error("ft_memory_document_source::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_memory_document_source::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_source::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_memory_document_source::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_data_pointer = ft_nullptr;
    this->_data_length = 0;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_memory_document_source::_state_destroyed;
    return (disable_error);
}

int ft_memory_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_memory_document_source::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_source::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_memory_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_memory_document_source::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_source::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_memory_document_source::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_memory_document_source::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_memory_document_source::set_data(const char *data_pointer,
    size_t data_length) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_memory_document_source::set_data");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_data_pointer = data_pointer;
    this->_data_length = data_length;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const char *ft_memory_document_source::get_data() const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_source::get_data");
    return (this->_data_pointer);
}

size_t ft_memory_document_source::get_length() const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_source::get_length");
    return (this->_data_length);
}

int ft_memory_document_source::read_all(ft_string &output)
{
    this->abort_if_not_initialized("ft_memory_document_source::read_all");
    if (this->_data_pointer == ft_nullptr && this->_data_length != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    output.assign(this->_data_pointer, this->_data_length);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_memory_document_source::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_source::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_memory_document_sink::ft_memory_document_sink() noexcept
    : _storage_pointer(ft_nullptr), _mutex(ft_nullptr),
      _initialized_state(ft_memory_document_sink::_state_uninitialized)
{
    return ;
}

ft_memory_document_sink::ft_memory_document_sink(ft_string *storage_pointer) noexcept
    : _storage_pointer(storage_pointer), _mutex(ft_nullptr),
      _initialized_state(ft_memory_document_sink::_state_uninitialized)
{
    return ;
}

ft_memory_document_sink::~ft_memory_document_sink()
{
    if (this->_initialized_state == ft_memory_document_sink::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_memory_document_sink::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_memory_document_sink lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_memory_document_sink::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_memory_document_sink::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_memory_document_sink::initialize() noexcept
{
    if (this->_initialized_state == ft_memory_document_sink::_state_initialized)
    {
        this->abort_lifecycle_error("ft_memory_document_sink::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_memory_document_sink::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_sink::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_memory_document_sink::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_storage_pointer = ft_nullptr;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_memory_document_sink::_state_destroyed;
    return (disable_error);
}

int ft_memory_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_memory_document_sink::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_sink::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_memory_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_memory_document_sink::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_memory_document_sink::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_memory_document_sink::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_memory_document_sink::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_memory_document_sink::set_storage(ft_string *storage_pointer) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_memory_document_sink::set_storage");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_storage_pointer = storage_pointer;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_string *ft_memory_document_sink::get_storage() const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_sink::get_storage");
    return (this->_storage_pointer);
}

int ft_memory_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    this->abort_if_not_initialized("ft_memory_document_sink::write_all");
    if (this->_storage_pointer == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    if (data_pointer == ft_nullptr && data_length != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    this->_storage_pointer->assign(data_pointer, data_length);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_memory_document_sink::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_memory_document_sink::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

#if NETWORKING_HAS_OPENSSL
ft_http_document_source::ft_http_document_source() noexcept
    : _host(), _path(), _port(), _use_ssl(false), _mutex(ft_nullptr),
      _initialized_state(ft_http_document_source::_state_uninitialized)
{
    return ;
}

ft_http_document_source::ft_http_document_source(const char *host,
    const char *path, bool use_ssl, const char *port) noexcept
    : _host(), _path(), _port(), _use_ssl(use_ssl), _mutex(ft_nullptr),
      _initialized_state(ft_http_document_source::_state_uninitialized)
{
    if (host != ft_nullptr)
        this->_host = host;
    if (path != ft_nullptr)
        this->_path = path;
    if (port != ft_nullptr)
        this->_port = port;
    return ;
}

ft_http_document_source::~ft_http_document_source()
{
    if (this->_initialized_state == ft_http_document_source::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_http_document_source::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_http_document_source lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_http_document_source::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_http_document_source::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_http_document_source::initialize() noexcept
{
    if (this->_initialized_state == ft_http_document_source::_state_initialized)
    {
        this->abort_lifecycle_error("ft_http_document_source::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_http_document_source::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_source::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_http_document_source::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_host.clear();
    this->_path.clear();
    this->_port.clear();
    this->_use_ssl = false;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_http_document_source::_state_destroyed;
    return (disable_error);
}

int ft_http_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_http_document_source::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_source::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_http_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_http_document_source::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_source::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_http_document_source::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_http_document_source::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_http_document_source::configure(const char *host, const char *path,
    bool use_ssl, const char *port) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_http_document_source::configure");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (host == ft_nullptr)
        this->_host.clear();
    else
        this->_host = host;
    if (path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = path;
    if (port == ft_nullptr)
        this->_port.clear();
    else
        this->_port = port;
    this->_use_ssl = use_ssl;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const char *ft_http_document_source::get_host() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::get_host");
    return (this->_host.c_str());
}

const char *ft_http_document_source::get_path() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::get_path");
    return (this->_path.c_str());
}

const char *ft_http_document_source::get_port() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::get_port");
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

bool ft_http_document_source::is_ssl_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::is_ssl_enabled");
    return (this->_use_ssl);
}

int ft_http_document_source::read_all(ft_string &output)
{
    ft_string response;
    const char *port_pointer;
    int request_status;

    this->abort_if_not_initialized("ft_http_document_source::read_all");
    if (this->_host.size() == 0 || this->_path.size() == 0)
        return (FT_ERR_INVALID_STATE);
    port_pointer = ft_nullptr;
    if (this->_port.size() > 0)
        port_pointer = this->_port.c_str();
    request_status = http_get(this->_host.c_str(), this->_path.c_str(), response,
        this->_use_ssl, port_pointer);
    if (request_status != 0)
        return (request_status);
    output = response;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_http_document_source::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_source::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

ft_http_document_sink::ft_http_document_sink() noexcept
    : _host(), _path(), _port(), _use_ssl(false), _mutex(ft_nullptr),
      _initialized_state(ft_http_document_sink::_state_uninitialized)
{
    return ;
}

ft_http_document_sink::ft_http_document_sink(const char *host, const char *path,
    bool use_ssl, const char *port) noexcept
    : _host(), _path(), _port(), _use_ssl(use_ssl), _mutex(ft_nullptr),
      _initialized_state(ft_http_document_sink::_state_uninitialized)
{
    if (host != ft_nullptr)
        this->_host = host;
    if (path != ft_nullptr)
        this->_path = path;
    if (port != ft_nullptr)
        this->_port = port;
    return ;
}

ft_http_document_sink::~ft_http_document_sink()
{
    if (this->_initialized_state == ft_http_document_sink::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_http_document_sink::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_http_document_sink lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_http_document_sink::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_http_document_sink::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_http_document_sink::initialize() noexcept
{
    if (this->_initialized_state == ft_http_document_sink::_state_initialized)
    {
        this->abort_lifecycle_error("ft_http_document_sink::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_http_document_sink::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_sink::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_http_document_sink::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_host.clear();
    this->_path.clear();
    this->_port.clear();
    this->_use_ssl = false;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_http_document_sink::_state_destroyed;
    return (disable_error);
}

int ft_http_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_http_document_sink::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_sink::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_http_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_http_document_sink::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_http_document_sink::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_http_document_sink::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_http_document_sink::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_http_document_sink::configure(const char *host, const char *path,
    bool use_ssl, const char *port) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_http_document_sink::configure");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (host == ft_nullptr)
        this->_host.clear();
    else
        this->_host = host;
    if (path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = path;
    if (port == ft_nullptr)
        this->_port.clear();
    else
        this->_port = port;
    this->_use_ssl = use_ssl;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const char *ft_http_document_sink::get_host() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::get_host");
    return (this->_host.c_str());
}

const char *ft_http_document_sink::get_path() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::get_path");
    return (this->_path.c_str());
}

const char *ft_http_document_sink::get_port() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::get_port");
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

bool ft_http_document_sink::is_ssl_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::is_ssl_enabled");
    return (this->_use_ssl);
}

int ft_http_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    ft_string body;
    ft_string response;
    const char *port_pointer;
    int request_status;

    this->abort_if_not_initialized("ft_http_document_sink::write_all");
    if (this->_host.size() == 0 || this->_path.size() == 0)
        return (FT_ERR_INVALID_STATE);
    if (data_pointer == ft_nullptr && data_length != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    body.assign(data_pointer, data_length);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    port_pointer = ft_nullptr;
    if (this->_port.size() > 0)
        port_pointer = this->_port.c_str();
    request_status = http_post(this->_host.c_str(), this->_path.c_str(), body,
        response, this->_use_ssl, port_pointer);
    if (request_status != 0)
        return (request_status);
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_http_document_sink::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_http_document_sink::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
#endif
