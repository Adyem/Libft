#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_internal.hpp"
#include "compatebility_sound_device_alsa.hpp"
#include <alsa/asoundlib.h>
#include <limits>
#include <new>
#include <pthread.h>
#include <string.h>

void *ft_sound_device_alsa::sound_thread(void *argument)
{
    ft_sound_device_alsa *sound_device;

    sound_device = static_cast<ft_sound_device_alsa *>(argument);
    while (sound_device->_is_running == FT_TRUE
        && sound_device->_current_clip != ft_nullptr
        && sound_device->_playback_pos < sound_device->_current_clip->get_size())
    {
        ft_size_t bytes_to_copy;

        bytes_to_copy = sound_device->_current_clip->get_size()
            - sound_device->_playback_pos;
        if (bytes_to_copy > static_cast<ft_size_t>(sound_device->_buffer_size))
            bytes_to_copy = static_cast<ft_size_t>(sound_device->_buffer_size);
        memcpy(sound_device->_buffer,
            sound_device->_current_clip->get_data() + sound_device->_playback_pos,
            bytes_to_copy);
        snd_pcm_writei(sound_device->_pcm_handle, sound_device->_buffer,
            bytes_to_copy / static_cast<ft_size_t>(sound_device->_spec.channels * 2));
        sound_device->_playback_pos += bytes_to_copy;
    }
    return (ft_nullptr);
}

ft_sound_device_alsa::ft_sound_device_alsa(void)
{
    this->_pcm_handle = ft_nullptr;
    this->_is_running = FT_FALSE;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_alsa::ft_sound_device_alsa(const ft_sound_device_alsa &other)
    : ft_sound_device(other)
{
    (void)other;
    this->_pcm_handle = ft_nullptr;
    this->_is_running = FT_FALSE;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_alsa::ft_sound_device_alsa(ft_sound_device_alsa &&other)
    : ft_sound_device(static_cast<ft_sound_device &&>(other))
{
    (void)other;
    this->_pcm_handle = ft_nullptr;
    this->_is_running = FT_FALSE;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_alsa::~ft_sound_device_alsa(void)
{
    this->close();
    return ;
}

int32_t ft_sound_device_alsa::open(const ft_sound_spec *spec)
{
    snd_pcm_hw_params_t *hardware_parameters;
    int32_t error_code;
    uint32_t frequency;
    snd_pcm_uframes_t buffer_size;

    if (spec == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->_spec = *spec;
    error_code = snd_pcm_open(&this->_pcm_handle, "default",
        SND_PCM_STREAM_PLAYBACK, 0);
    if (error_code < 0)
        return (FT_ERR_IO);
    hardware_parameters = ft_nullptr;
    snd_pcm_hw_params_alloca(&hardware_parameters);
    snd_pcm_hw_params_any(this->_pcm_handle, hardware_parameters);
    snd_pcm_hw_params_set_access(this->_pcm_handle, hardware_parameters,
        SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(this->_pcm_handle, hardware_parameters,
        SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(this->_pcm_handle, hardware_parameters,
        this->_spec.channels);
    frequency = this->_spec.freq;
    snd_pcm_hw_params_set_rate_near(this->_pcm_handle, hardware_parameters,
        &frequency, ft_nullptr);
    this->_spec.freq = frequency;
    buffer_size = this->_spec.samples;
    snd_pcm_hw_params_set_period_size_near(this->_pcm_handle,
        hardware_parameters, &buffer_size, ft_nullptr);
    if (buffer_size > static_cast<snd_pcm_uframes_t>(FT_INT32_MAX))
    {
        snd_pcm_close(this->_pcm_handle);
        return (FT_ERR_IO);
    }
    this->_spec.samples = static_cast<int32_t>(buffer_size);
    error_code = snd_pcm_hw_params(this->_pcm_handle, hardware_parameters);
    if (error_code < 0)
    {
        snd_pcm_close(this->_pcm_handle);
        return (FT_ERR_IO);
    }
    this->_buffer_size = this->_spec.samples * this->_spec.channels * 2;
    this->_buffer = new (std::nothrow) uint8_t[this->_buffer_size];
    if (this->_buffer == ft_nullptr)
    {
        snd_pcm_close(this->_pcm_handle);
        return (FT_ERR_NO_MEMORY);
    }
    this->_is_running = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void ft_sound_device_alsa::close(void)
{
    this->stop();
    if (this->_is_running == FT_TRUE)
        this->_is_running = FT_FALSE;
    if (this->_pcm_handle != ft_nullptr)
    {
        snd_pcm_close(this->_pcm_handle);
        this->_pcm_handle = ft_nullptr;
    }
    if (this->_buffer != ft_nullptr)
    {
        delete[] this->_buffer;
        this->_buffer = ft_nullptr;
    }
    return ;
}

void ft_sound_device_alsa::pause(int32_t pause_on)
{
    if (this->_pcm_handle == ft_nullptr)
        return ;
    snd_pcm_pause(this->_pcm_handle, pause_on);
    return ;
}

void ft_sound_device_alsa::play(const ft_sound_clip *clip)
{
    if (clip == ft_nullptr || this->_is_running == FT_FALSE)
        return ;
    this->stop();
    this->_current_clip = clip;
    this->_playback_pos = 0;
    pthread_create(&this->_playback_thread, ft_nullptr,
        ft_sound_device_alsa::sound_thread, this);
    return ;
}

void ft_sound_device_alsa::stop(void)
{
    if (this->_current_clip != ft_nullptr)
    {
        pthread_join(this->_playback_thread, ft_nullptr);
        this->_current_clip = ft_nullptr;
        this->_playback_pos = 0;
    }
    return ;
}

ft_sound_device *cmp_create_sound_device(void)
{
    return (new ft_sound_device_alsa());
}

#endif
