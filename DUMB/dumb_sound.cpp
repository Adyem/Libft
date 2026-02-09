#include "dumb_sound.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"

ft_sound_device::ft_sound_device(void)
{
    return ;
}

ft_sound_device::~ft_sound_device(void)
{
    this->disable_thread_safety();
    return ;
}

int ft_sound_device::prepare_thread_safety(void) noexcept
{
    if (this->_mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_sound_device::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_sound_device::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void ft_sound_device::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_sound_device::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_sound_device::runtime_mutex(void)
{
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
#if defined(_WIN32)
    return (ft_create_sound_device_win32());
#elif defined(__APPLE__)
    return (ft_create_sound_device_coreaudio());
#else
    return (ft_create_sound_device_alsa());
#endif
}
