#include "sound_clip.hpp"
#include "dumb_io.hpp"
#include "../CMA/CMA.hpp"
#include "sound_device.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include <string.h>
#include <new>

static int32_t create_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    pt_recursive_mutex *created_mutex;
    int32_t initialize_error;

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

static uint32_t destroy_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    uint32_t destroy_error;

    if (mutex_pointer == ft_nullptr || *mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = (*mutex_pointer)->destroy();
    delete *mutex_pointer;
    *mutex_pointer = ft_nullptr;
    return (destroy_error);
}

static int32_t lock_ordered_mutexes(pt_recursive_mutex *mutex_left,
    pt_recursive_mutex *mutex_right, pt_recursive_mutex **first_mutex,
    pt_recursive_mutex **second_mutex)
{
    int32_t lock_error;

    if (reinterpret_cast<uintptr_t>(mutex_left)
        <= reinterpret_cast<uintptr_t>(mutex_right))
    {
        *first_mutex = mutex_left;
        *second_mutex = mutex_right;
    }
    else
    {
        *first_mutex = mutex_right;
        *second_mutex = mutex_left;
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(*first_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (*second_mutex == *first_mutex)
        return (FT_ERR_SUCCESS);
    lock_error = pt_recursive_mutex_lock_if_not_null(*second_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(*first_mutex);
        return (lock_error);
    }
    return (FT_ERR_SUCCESS);
}

static void unlock_ordered_mutexes(pt_recursive_mutex *first_mutex,
    pt_recursive_mutex *second_mutex)
{
    if (second_mutex != first_mutex)
        (void)pt_recursive_mutex_unlock_if_not_null(second_mutex);
    (void)pt_recursive_mutex_unlock_if_not_null(first_mutex);
    return ;
}

ft_sound_clip::ft_sound_clip(void)
    : _spec(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_sound_clip::~ft_sound_clip(void)
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_sound_clip::initialize(void)
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_sound_clip::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_data.clear();
    this->_spec = new (std::nothrow) ft_sound_spec();
    if (this->_spec == ft_nullptr)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_NO_MEMORY);
    }
    this->_spec->freq = 44100;
    this->_spec->channels = 2;
    this->_spec->samples = 4096;
    this->_spec->callback = ft_nullptr;
    this->_spec->userdata = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_sound_clip::initialize(const ft_sound_clip &other)
{
    int32_t lock_error;
    uint32_t destroy_error;
    pt_recursive_mutex *first_mutex;
    pt_recursive_mutex *second_mutex;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_clip::initialize(const ft_sound_clip &) source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (destroy_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_clip::initialize(const ft_sound_clip &) source",
            "called with source object that is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    lock_error = lock_ordered_mutexes(this->_mutex, other._mutex,
        &first_mutex, &second_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    this->_data = other._data;
    if (other._spec != ft_nullptr && this->_spec != ft_nullptr)
        *this->_spec = *other._spec;
    unlock_ordered_mutexes(first_mutex, second_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_sound_clip::initialize(ft_sound_clip &&other)
{
    uint32_t initialization_error;
    uint32_t move_error;
    uint32_t destroy_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_clip::initialize(ft_sound_clip &&) source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (destroy_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_clip::initialize(ft_sound_clip &&) source",
            "called with source object that is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
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

int32_t ft_sound_clip::destroy(void)
{
    uint32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_data.clear();
    delete this->_spec;
    this->_spec = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_sound_clip::move(ft_sound_clip &other)
{
    int32_t first_lock_error;
    int32_t second_lock_error;
    int32_t destroy_error;
    pt_recursive_mutex *first_mutex;
    pt_recursive_mutex *second_mutex;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_sound_clip::move source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
            return (this->destroy());
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_sound_clip::move source",
            "called with source object that is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        if (this->initialize() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    if (reinterpret_cast<uintptr_t>(this->_mutex)
        < reinterpret_cast<uintptr_t>(other._mutex))
    {
        first_mutex = this->_mutex;
        second_mutex = other._mutex;
    }
    else
    {
        first_mutex = other._mutex;
        second_mutex = this->_mutex;
    }
    first_lock_error = pt_recursive_mutex_lock_if_not_null(first_mutex);
    if (first_lock_error != FT_ERR_SUCCESS)
    {
        this->_data.clear();
        if (this->_spec != ft_nullptr)
        {
            delete this->_spec;
            this->_spec = ft_nullptr;
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (first_lock_error);
    }
    if (second_mutex != first_mutex)
        second_lock_error = pt_recursive_mutex_lock_if_not_null(second_mutex);
    else
        second_lock_error = FT_ERR_SUCCESS;
    if (second_lock_error != FT_ERR_SUCCESS)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(first_mutex);
        this->_data.clear();
        if (this->_spec != ft_nullptr)
        {
            delete this->_spec;
            this->_spec = ft_nullptr;
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (second_lock_error);
    }
    this->_data.swap(other._data);
    if (this->_spec != ft_nullptr && other._spec != ft_nullptr)
    {
        ft_sound_spec temporary_spec;

        temporary_spec = *this->_spec;
        *this->_spec = *other._spec;
        *other._spec = temporary_spec;
    }
    if (second_mutex != first_mutex)
        (void)pt_recursive_mutex_unlock_if_not_null(second_mutex);
    (void)pt_recursive_mutex_unlock_if_not_null(first_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_sound_clip::prepare_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_clip::prepare_thread_safety");
    if (this->_mutex)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int32_t mutex_error;

    mutex_error = create_recursive_mutex(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

uint32_t ft_sound_clip::teardown_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_clip::teardown_thread_safety");
    return (destroy_recursive_mutex(&this->_mutex));
}

int32_t ft_sound_clip::load_wav(const char *file_path)
{
    int32_t lock_error;
    char *buffer;
    ft_size_t size;
    uint32_t fmt_size;
    char *data_chunk;
    ft_size_t data_offset;
    uint32_t data_size;

    buffer = ft_nullptr;
    size = 0;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_sound_clip::load_wav");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (ft_read_file(file_path, &buffer, &size) != FT_ERR_SUCCESS)
    {
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_IO);
    }

    if (size < 44)
    {
        cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }

    if (strncmp(buffer, "RIFF", 4) != 0 || strncmp(buffer + 8, "WAVE", 4) != 0)
    {
        cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }

    if (strncmp(buffer + 12, "fmt ", 4) != 0)
    {
        cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }

    fmt_size = *reinterpret_cast<const uint32_t *>(buffer + 16);
    if (fmt_size < 16)
    {
        cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }

    this->_spec->channels = *reinterpret_cast<const uint16_t *>(buffer + 22);
    this->_spec->freq = *reinterpret_cast<const uint32_t *>(buffer + 24);

    data_chunk = strstr(buffer, "data");
    if (data_chunk == ft_nullptr)
    {
        cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }

    data_offset = static_cast<ft_size_t>(data_chunk - buffer);
    data_size = *reinterpret_cast<const uint32_t *>(buffer + data_offset + 4);

    this->_data.assign(buffer + data_offset + 8, buffer + data_offset + 8 + data_size);

    cma_free(buffer);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

uint32_t ft_sound_clip::enable_thread_safety() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_clip::enable_thread_safety");
    return (this->prepare_thread_safety());
}

uint32_t ft_sound_clip::disable_thread_safety() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_clip::disable_thread_safety");
    return (this->teardown_thread_safety());
}

ft_bool ft_sound_clip::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_clip::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

const uint8_t *ft_sound_clip::get_data(void) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_sound_clip::get_data");
    return (this->_data.data());
}

ft_size_t ft_sound_clip::get_size(void) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_sound_clip::get_size");
    return (this->_data.size());
}

const ft_sound_spec *ft_sound_clip::get_spec(void) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_sound_clip::get_spec");
    return (this->_spec);
}
