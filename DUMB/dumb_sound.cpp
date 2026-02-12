#include "dumb_sound.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
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

ft_sound_device::ft_sound_device(void)
    : _initialized_state(ft_sound_device::_state_uninitialized)
{
    return ;
}

ft_sound_device::~ft_sound_device(void)
{
    if (this->_initialized_state == ft_sound_device::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_sound_device::~ft_sound_device",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_sound_device::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_sound_device::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_sound_device lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_sound_device::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_sound_device::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_sound_device::initialize(void)
{
    if (this->_initialized_state == ft_sound_device::_state_initialized)
    {
        this->abort_lifecycle_error("ft_sound_device::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_sound_device::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_sound_device::initialize(const ft_sound_device &other)
{
    if (other._initialized_state == ft_sound_device::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_device::initialize(const ft_sound_device &) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_device::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    return (this->initialize());
}

int ft_sound_device::initialize(ft_sound_device &&other)
{
    if (other._initialized_state == ft_sound_device::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_sound_device::initialize(ft_sound_device &&) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_device::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    return (this->move(other));
}

int ft_sound_device::destroy(void)
{
    if (this->_initialized_state != ft_sound_device::_state_initialized)
    {
        this->abort_lifecycle_error("ft_sound_device::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->disable_thread_safety();
    this->_initialized_state = ft_sound_device::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_sound_device::move(ft_sound_device &other)
{
    this->abort_if_not_initialized("ft_sound_device::move destination");
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialized_state == ft_sound_device::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_sound_device::move source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_sound_device::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
        this->disable_thread_safety();
    if (other._mutex != ft_nullptr)
    {
        this->_mutex = other._mutex;
        other._mutex = ft_nullptr;
    }
    other._initialized_state = ft_sound_device::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_sound_device::prepare_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_sound_device::prepare_thread_safety");
    if (this->_mutex)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = create_recursive_mutex(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void ft_sound_device::teardown_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_sound_device::teardown_thread_safety");
    destroy_recursive_mutex(&this->_mutex);
    return ;
}

int ft_sound_device::enable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_sound_device::enable_thread_safety");
    return (this->prepare_thread_safety());
}

void ft_sound_device::disable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_sound_device::disable_thread_safety");
    this->teardown_thread_safety();
    return ;
}

bool ft_sound_device::is_thread_safe_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_sound_device::is_thread_safe_enabled");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_sound_device::runtime_mutex(void)
{
    this->abort_if_not_initialized("ft_sound_device::runtime_mutex");
    if (!this->_mutex)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

#if defined(_WIN32)
ft_sound_device *ft_create_sound_device_win32(void);
#elif defined(__APPLE__)
ft_sound_device *ft_create_sound_device_coreaudio(void);
#else
ft_sound_device *ft_create_sound_device_alsa(void);
#endif

ft_sound_device *ft_create_sound_device(void)
{
    ft_sound_device *sound_device;

#if defined(_WIN32)
    sound_device = ft_create_sound_device_win32();
#elif defined(__APPLE__)
    sound_device = ft_create_sound_device_coreaudio();
#else
    sound_device = ft_create_sound_device_alsa();
#endif
    if (sound_device != ft_nullptr)
        (void)sound_device->initialize();
    return (sound_device);
}
