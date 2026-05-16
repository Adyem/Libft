#include "sound_device.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
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

ft_sound_device::ft_sound_device(void)
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_sound_device::~ft_sound_device(void)
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_sound_device::initialize(void)
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_sound_device::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_sound_device::initialize(const ft_sound_device &other)
{
    uint32_t destroy_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_device::initialize(const ft_sound_device &) source",
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
            "ft_sound_device::initialize(const ft_sound_device &) source",
            "called with source object that is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    return (this->initialize());
}

int32_t ft_sound_device::initialize(ft_sound_device &&other)
{
    uint32_t destroy_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_sound_device::initialize(ft_sound_device &&) source",
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
            "ft_sound_device::initialize(ft_sound_device &&) source",
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
    return (this->move(other));
}

int32_t ft_sound_device::destroy(void)
{
    uint32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t ft_sound_device::move(ft_sound_device &other)
{
    int32_t lock_error;
    int32_t destroy_error;
    pt_recursive_mutex *first_mutex;
    pt_recursive_mutex *second_mutex;
    pt_recursive_mutex *old_destination_mutex;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_sound_device::move source",
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
        errno_abort_lifecycle(other._initialised_state, "ft_sound_device::move source",
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
    lock_error = lock_ordered_mutexes(this->_mutex, other._mutex,
        &first_mutex, &second_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    old_destination_mutex = this->_mutex;
    this->_mutex = other._mutex;
    other._mutex = ft_nullptr;
    unlock_ordered_mutexes(first_mutex, second_mutex);
    if (old_destination_mutex != ft_nullptr
        && old_destination_mutex != this->_mutex)
    {
        (void)destroy_recursive_mutex(&old_destination_mutex);
    }
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_sound_device::prepare_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_device::prepare_thread_safety");
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

uint32_t ft_sound_device::teardown_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_device::teardown_thread_safety");
    return (destroy_recursive_mutex(&this->_mutex));
}

uint32_t ft_sound_device::enable_thread_safety() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_device::enable_thread_safety");
    return (this->prepare_thread_safety());
}

uint32_t ft_sound_device::disable_thread_safety() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_device::disable_thread_safety");
    return (this->teardown_thread_safety());
}

ft_bool ft_sound_device::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_sound_device::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ft_sound_device *ft_create_sound_device(void)
{
    ft_sound_device *sound_device;

    sound_device = cmp_create_sound_device();
    if (sound_device != ft_nullptr)
        (void)sound_device->initialize();
    return (sound_device);
}
