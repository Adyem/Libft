#include "dumb_sound_clip.hpp"
#include "dumb_io.hpp"
#include "../CMA/CMA.hpp"
#include "dumb_sound.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <string.h>
#include <new>

static int create_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    pt_recursive_mutex *created_mutex;
    int initialize_error;

    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    created_mutex = new (std::nothrow) pt_recursive_mutex();
    if (created_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = created_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete created_mutex;
        return (initialize_error);
    }
    *mutex_pointer = created_mutex;
    return (FT_ERR_SUCCESS);
}

static void destroy_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    int destroy_error;

    if (mutex_pointer == ft_nullptr || *mutex_pointer == ft_nullptr)
        return ;
    destroy_error = (*mutex_pointer)->destroy();
    if (destroy_error == FT_ERR_SUCCESS)
    {
        delete *mutex_pointer;
        *mutex_pointer = ft_nullptr;
    }
    return ;
}

static int lock_recursive_mutex_if_valid(pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->lock());
}

static int unlock_recursive_mutex_if_valid(pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->unlock());
}

ft_sound_clip::ft_sound_clip(void)
    : _spec(ft_nullptr)
    , _initialized_state(ft_sound_clip::_state_uninitialized)
{
    return ;
}

ft_sound_clip::~ft_sound_clip(void)
{
    if (this->_initialized_state == ft_sound_clip::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_sound_clip::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_sound_clip lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_sound_clip::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_sound_clip::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_sound_clip::initialize(void)
{
    if (this->_initialized_state == ft_sound_clip::_state_initialized)
    {
        this->abort_lifecycle_error("ft_sound_clip::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_data.clear();
    this->_spec = new (std::nothrow) ft_sound_spec();
    if (this->_spec == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    this->_spec->freq = 44100;
    this->_spec->channels = 2;
    this->_spec->samples = 4096;
    this->_spec->callback = ft_nullptr;
    this->_spec->userdata = ft_nullptr;
    this->_initialized_state = ft_sound_clip::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_sound_clip::initialize(const ft_sound_clip &other)
{
    int lock_error;
    int unlock_error;

    if (other._initialized_state == ft_sound_clip::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_clip::initialize(const ft_sound_clip &) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_clip::_state_initialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_clip::initialize(const ft_sound_clip &) source",
            "called with source object that is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    lock_error = lock_recursive_mutex_if_valid(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    this->_data = other._data;
    if (other._spec != ft_nullptr && this->_spec != ft_nullptr)
        *this->_spec = *other._spec;
    unlock_error = unlock_recursive_mutex_if_valid(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_sound_clip::initialize(ft_sound_clip &&other)
{
    int initialization_error;
    int move_error;

    if (other._initialized_state == ft_sound_clip::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_clip::initialize(ft_sound_clip &&) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_clip::_state_initialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_clip::initialize(ft_sound_clip &&) source",
            "called with source object that is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_sound_clip::destroy(void)
{
    if (this->_initialized_state != ft_sound_clip::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_data.clear();
    delete this->_spec;
    this->_spec = ft_nullptr;
    this->disable_thread_safety();
    this->_initialized_state = ft_sound_clip::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_sound_clip::move(ft_sound_clip &other)
{
    int this_lock_error;
    int other_lock_error;
    int this_unlock_error;
    int other_unlock_error;

    if (other._initialized_state == ft_sound_clip::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_sound_clip::move source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_clip::_state_initialized)
    {
        other.abort_lifecycle_error("ft_sound_clip::move source",
            "called with source object that is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != ft_sound_clip::_state_initialized)
    {
        if (this->initialize() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    this_lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (this_lock_error != FT_ERR_SUCCESS)
        return (this_lock_error);
    other_lock_error = lock_recursive_mutex_if_valid(other._mutex);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this_unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (this_unlock_error != FT_ERR_SUCCESS)
            return (this_unlock_error);
        return (other_lock_error);
    }
    this->_data.swap(other._data);
    if (this->_spec != ft_nullptr && other._spec != ft_nullptr)
    {
        ft_sound_spec temporary_spec;

        temporary_spec = *this->_spec;
        *this->_spec = *other._spec;
        *other._spec = temporary_spec;
    }
    other_unlock_error = unlock_recursive_mutex_if_valid(other._mutex);
    if (other_unlock_error != FT_ERR_SUCCESS)
        return (other_unlock_error);
    this_unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (this_unlock_error != FT_ERR_SUCCESS)
        return (this_unlock_error);
    return (FT_ERR_SUCCESS);
}

int ft_sound_clip::prepare_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_sound_clip::prepare_thread_safety");
    if (this->_mutex)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = create_recursive_mutex(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void ft_sound_clip::teardown_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_sound_clip::teardown_thread_safety");
    destroy_recursive_mutex(&this->_mutex);
    return ;
}

int ft_sound_clip::load_wav(const char *file_path)
{
    int lock_error;
    int unlock_error;
    char *buffer = NULL;
    size_t size = 0;

    this->abort_if_not_initialized("ft_sound_clip::load_wav");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (ft_read_file(file_path, &buffer, &size) != ft_io_ok)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_platform_failure);
    }

    if (size < 44)
    {
        cma_free(buffer);
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_invalid_argument);
    }

    if (strncmp(buffer, "RIFF", 4) != 0 || strncmp(buffer + 8, "WAVE", 4) != 0)
    {
        cma_free(buffer);
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_invalid_argument);
    }

    if (strncmp(buffer + 12, "fmt ", 4) != 0)
    {
        cma_free(buffer);
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_invalid_argument);
    }

    uint32_t fmt_size = *reinterpret_cast<const uint32_t *>(buffer + 16);
    if (fmt_size < 16)
    {
        cma_free(buffer);
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_invalid_argument);
    }

    this->_spec->channels = *reinterpret_cast<const uint16_t *>(buffer + 22);
    this->_spec->freq = *reinterpret_cast<const uint32_t *>(buffer + 24);

    char *data_chunk = strstr(buffer, "data");
    if (data_chunk == NULL)
    {
        cma_free(buffer);
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_sound_error_invalid_argument);
    }

    size_t data_offset = data_chunk - buffer;
    uint32_t data_size = *reinterpret_cast<const uint32_t *>(buffer + data_offset + 4);

    _data.assign(buffer + data_offset + 8, buffer + data_offset + 8 + data_size);

    cma_free(buffer);
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_sound_ok);
}

int ft_sound_clip::enable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_sound_clip::enable_thread_safety");
    return (this->prepare_thread_safety());
}

void ft_sound_clip::disable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_sound_clip::disable_thread_safety");
    this->teardown_thread_safety();
    return ;
}

bool ft_sound_clip::is_thread_safe_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_sound_clip::is_thread_safe_enabled");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_sound_clip::runtime_mutex(void)
{
    this->abort_if_not_initialized("ft_sound_clip::runtime_mutex");
    if (!this->_mutex)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

const uint8_t *ft_sound_clip::get_data(void) const
{
    this->abort_if_not_initialized("ft_sound_clip::get_data");
    return (_data.data());
}

size_t ft_sound_clip::get_size(void) const
{
    this->abort_if_not_initialized("ft_sound_clip::get_size");
    return (_data.size());
}

const ft_sound_spec *ft_sound_clip::get_spec(void) const
{
    this->abort_if_not_initialized("ft_sound_clip::get_spec");
    return (this->_spec);
}
