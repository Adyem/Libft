#include "dumb_sound.hpp"

ft_sound_device::ft_sound_device(void)
{
    return ;
}

ft_sound_device::~ft_sound_device(void)
{
    return ;
}

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
