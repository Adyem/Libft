#include "document_backend.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CMA/CMA.hpp"
#include "../Networking/http_client.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include "../Observability/observability.hpp"
#include <fcntl.h>
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

static void parser_observability_emit(const char *operation, const char *resource,
    ft_observability_trace_phase phase, int32_t error_code,
    ft_size_t bytes_read, ft_size_t bytes_written)
{
    ft_observability_trace_event event;
    ft_bool success;

    event.module = FT_OBSERVABILITY_MODULE_PARSER;
    event.phase = phase;
    event.operation = operation;
    event.resource = resource;
    event.error_code = error_code;
    event.bytes_read = bytes_read;
    event.bytes_written = bytes_written;
    (void)observability_trace_emit(&event);
    if (phase == FT_OBSERVABILITY_TRACE_FINISH)
    {
        success = FT_FALSE;
        if (error_code == FT_ERR_SUCCESS)
            success = FT_TRUE;
        (void)observability_record_operation(FT_OBSERVABILITY_MODULE_PARSER,
            success, bytes_read, bytes_written);
    }
    return ;
}

ft_document_source::ft_document_source() noexcept
    : _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_document_source::~ft_document_source()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_document_source::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_document_source::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_source::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_source::enable_thread_safety");
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

int32_t ft_document_source::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_document_source::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_source::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_source::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_document_source::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}


ft_document_sink::ft_document_sink() noexcept
    : _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_document_sink::~ft_document_sink()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_document_sink::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_document_sink::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_sink::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_sink::enable_thread_safety");
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

int32_t ft_document_sink::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_document_sink::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_sink::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_document_sink::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_document_sink::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}


ft_file_document_source::ft_file_document_source() noexcept
    : _path(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_file_document_source::~ft_file_document_source()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_file_document_source::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_file_document_source::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_source::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_path.clear();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_file_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::enable_thread_safety");
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

int32_t ft_file_document_source::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_file_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_file_document_source::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_source::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_source::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_file_document_source::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_file_document_source::set_path(const char *file_path) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::set_path");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::get_path");
    return (this->_path.c_str());
}

int32_t ft_file_document_source::read_all(ft_string &output)
{
    su_file *file_stream;
    int64_t file_size_long;
    ft_size_t file_size;
    char *content_pointer;
    ft_size_t read_count;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_source::read_all");
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
        if (file_size_long > static_cast<int64_t>(static_cast<ft_size_t>(-1)))
            error_code = FT_ERR_OUT_OF_RANGE;
    }
    if (error_code == FT_ERR_SUCCESS)
    {
        file_size = static_cast<ft_size_t>(file_size_long);
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
    if (output.get_error() != FT_ERR_SUCCESS)
        return (output.get_error());
    return (FT_ERR_SUCCESS);
}


ft_file_document_sink::ft_file_document_sink() noexcept
    : _path(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_file_document_sink::~ft_file_document_sink()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_file_document_sink::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_file_document_sink::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_sink::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_path.clear();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_file_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::enable_thread_safety");
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

int32_t ft_file_document_sink::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_file_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_file_document_sink::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_sink::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_document_sink::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_file_document_sink::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_file_document_sink::set_path(const char *file_path) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::set_path");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::get_path");
    return (this->_path.c_str());
}

int32_t ft_file_document_sink::write_all(const char *data_pointer, ft_size_t data_length)
{
    su_file *file_stream;
    ft_size_t written_count;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_file_document_sink::write_all");
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

ft_memory_document_source::ft_memory_document_source() noexcept
    : _data_pointer(ft_nullptr), _data_length(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_memory_document_source::~ft_memory_document_source()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_memory_document_source::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_memory_document_source::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_source::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_data_pointer = ft_nullptr;
    this->_data_length = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_memory_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::enable_thread_safety");
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

int32_t ft_memory_document_source::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_memory_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_memory_document_source::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_source::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_source::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_memory_document_source::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_memory_document_source::set_data(const char *data_pointer,
    ft_size_t data_length) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::set_data");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::get_data");
    return (this->_data_pointer);
}

ft_size_t ft_memory_document_source::get_length() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::get_length");
    return (this->_data_length);
}

int32_t ft_memory_document_source::read_all(ft_string &output)
{
    int32_t error_code;
    ft_size_t bytes_read;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_source::read_all");
    parser_observability_emit("memory_source_read_all", "memory",
        FT_OBSERVABILITY_TRACE_START, FT_ERR_SUCCESS, 0, 0);
    if (this->_data_pointer == ft_nullptr && this->_data_length != 0)
    {
        parser_observability_emit("memory_source_read_all", "memory",
            FT_OBSERVABILITY_TRACE_FINISH, FT_ERR_INVALID_ARGUMENT, 0, 0);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    output.assign(this->_data_pointer, this->_data_length);
    error_code = output.get_error();
    bytes_read = 0;
    if (error_code == FT_ERR_SUCCESS)
        bytes_read = this->_data_length;
    parser_observability_emit("memory_source_read_all", "memory",
        FT_OBSERVABILITY_TRACE_FINISH, error_code, bytes_read, 0);
    return (error_code);
}


ft_memory_document_sink::ft_memory_document_sink() noexcept
    : _storage_pointer(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_memory_document_sink::~ft_memory_document_sink()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_memory_document_sink::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_memory_document_sink::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_sink::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_storage_pointer = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_memory_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::enable_thread_safety");
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

int32_t ft_memory_document_sink::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_memory_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_memory_document_sink::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_sink::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_memory_document_sink::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_memory_document_sink::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_memory_document_sink::set_storage(ft_string *storage_pointer) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::set_storage");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::get_storage");
    return (this->_storage_pointer);
}

int32_t ft_memory_document_sink::write_all(const char *data_pointer, ft_size_t data_length)
{
    int32_t error_code;
    ft_size_t bytes_written;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_memory_document_sink::write_all");
    parser_observability_emit("memory_sink_write_all", "memory",
        FT_OBSERVABILITY_TRACE_START, FT_ERR_SUCCESS, 0, 0);
    if (this->_storage_pointer == ft_nullptr)
    {
        parser_observability_emit("memory_sink_write_all", "memory",
            FT_OBSERVABILITY_TRACE_FINISH, FT_ERR_INVALID_STATE, 0, 0);
        return (FT_ERR_INVALID_STATE);
    }
    if (data_pointer == ft_nullptr && data_length != 0)
    {
        parser_observability_emit("memory_sink_write_all", "memory",
            FT_OBSERVABILITY_TRACE_FINISH, FT_ERR_INVALID_ARGUMENT, 0, 0);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_storage_pointer->assign(data_pointer, data_length);
    error_code = this->_storage_pointer->get_error();
    bytes_written = 0;
    if (error_code == FT_ERR_SUCCESS)
        bytes_written = data_length;
    parser_observability_emit("memory_sink_write_all", "memory",
        FT_OBSERVABILITY_TRACE_FINISH, error_code, 0, bytes_written);
    return (error_code);
}


#if NETWORKING_HAS_OPENSSL
ft_http_document_source::ft_http_document_source() noexcept
    : _host(), _path(), _port(), _use_ssl(false), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_http_document_source::~ft_http_document_source()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_http_document_source::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_http_document_source::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_source::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_host.clear();
    this->_path.clear();
    this->_port.clear();
    this->_use_ssl = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_http_document_source::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::enable_thread_safety");
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

int32_t ft_http_document_source::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_http_document_source::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_http_document_source::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_source::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_source::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_http_document_source::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_http_document_source::configure(const char *host, const char *path,
    ft_bool use_ssl, const char *port) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::configure");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::get_host");
    return (this->_host.c_str());
}

const char *ft_http_document_source::get_path() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::get_path");
    return (this->_path.c_str());
}

const char *ft_http_document_source::get_port() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::get_port");
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

ft_bool ft_http_document_source::is_ssl_enabled() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::is_ssl_enabled");
    return (this->_use_ssl);
}

int32_t ft_http_document_source::read_all(ft_string &output)
{
    ft_string response;
    const char *port_pointer;
    int32_t request_status;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_source::read_all");
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
    if (output.get_error() != FT_ERR_SUCCESS)
        return (output.get_error());
    return (FT_ERR_SUCCESS);
}


ft_http_document_sink::ft_http_document_sink() noexcept
    : _host(), _path(), _port(), _use_ssl(false), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_http_document_sink::~ft_http_document_sink()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


int32_t ft_http_document_sink::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_http_document_sink::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_sink::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_host.clear();
    this->_path.clear();
    this->_port.clear();
    this->_use_ssl = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_http_document_sink::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::enable_thread_safety");
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

int32_t ft_http_document_sink::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_http_document_sink::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_http_document_sink::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_sink::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_http_document_sink::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_http_document_sink::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_http_document_sink::configure(const char *host, const char *path,
    ft_bool use_ssl, const char *port) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::configure");
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
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::get_host");
    return (this->_host.c_str());
}

const char *ft_http_document_sink::get_path() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::get_path");
    return (this->_path.c_str());
}

const char *ft_http_document_sink::get_port() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::get_port");
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

ft_bool ft_http_document_sink::is_ssl_enabled() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::is_ssl_enabled");
    return (this->_use_ssl);
}

int32_t ft_http_document_sink::write_all(const char *data_pointer, ft_size_t data_length)
{
    ft_string body;
    ft_string response;
    const char *port_pointer;
    int32_t request_status;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_http_document_sink::write_all");
    if (this->_host.size() == 0 || this->_path.size() == 0)
        return (FT_ERR_INVALID_STATE);
    if (data_pointer == ft_nullptr && data_length != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    body.assign(data_pointer, data_length);
    if (body.get_error() != FT_ERR_SUCCESS)
        return (body.get_error());
    port_pointer = ft_nullptr;
    if (this->_port.size() > 0)
        port_pointer = this->_port.c_str();
    request_status = http_post(this->_host.c_str(), this->_path.c_str(), body,
        response, this->_use_ssl, port_pointer);
    if (request_status != 0)
        return (request_status);
    return (FT_ERR_SUCCESS);
}

#endif
