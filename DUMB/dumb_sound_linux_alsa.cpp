#if !defined(_WIN32) && !defined(__APPLE__)

#include "dumb_sound_linux_alsa.hpp"
#include "dumb_sound_internal.hpp"
#include "dumb_sound_clip.hpp"
#include <unistd.h>
#include <string.h>
#include <limits>

ft_sound_device *ft_create_sound_device_alsa(void);

void *ft_sound_device_alsa::sound_thread(void *argument)
{
    ft_sound_device_alsa *sound_device;

    sound_device = static_cast<ft_sound_device_alsa *>(argument);
    while (sound_device->_is_running && sound_device->_current_clip != NULL
        && sound_device->_playback_pos < sound_device->_current_clip->get_size())
    {
        size_t bytes_to_copy;

        bytes_to_copy = sound_device->_current_clip->get_size()
            - sound_device->_playback_pos;
        if (bytes_to_copy > static_cast<size_t>(sound_device->_buffer_size))
        {
            bytes_to_copy = sound_device->_buffer_size;
        }
        memcpy(sound_device->_buffer,
            sound_device->_current_clip->get_data() + sound_device->_playback_pos,
            bytes_to_copy);
        snd_pcm_writei(sound_device->_pcm_handle, sound_device->_buffer,
            bytes_to_copy / (sound_device->_spec.channels * 2));
        sound_device->_playback_pos += bytes_to_copy;
    }
    return (NULL);
}

ft_sound_device_alsa::ft_sound_device_alsa(void)
{
    this->_pcm_handle = NULL;
    this->_is_running = false;
    this->_buffer = NULL;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = NULL;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_alsa::~ft_sound_device_alsa(void)
{
    this->close();
    return ;
}

int ft_sound_device_alsa::open(const ft_sound_spec *spec)
{
    snd_pcm_hw_params_t *hardware_parameters;
    int error_code;

    if (spec == NULL)
    {
        return (ft_sound_error_invalid_argument);
    }
    this->_spec = *spec;
    error_code = snd_pcm_open(&this->_pcm_handle, "default",
        SND_PCM_STREAM_PLAYBACK, 0);
    if (error_code < 0)
    {
        return (ft_sound_error_platform_failure);
    }
    hardware_parameters = NULL;
    snd_pcm_hw_params_alloca(&hardware_parameters);
    snd_pcm_hw_params_any(this->_pcm_handle, hardware_parameters);
    snd_pcm_hw_params_set_access(this->_pcm_handle, hardware_parameters,
        SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(this->_pcm_handle, hardware_parameters,
        SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(this->_pcm_handle, hardware_parameters,
        this->_spec.channels);
    unsigned int frequency;

    frequency = this->_spec.freq;
    snd_pcm_hw_params_set_rate_near(this->_pcm_handle, hardware_parameters,
        &frequency, NULL);
    this->_spec.freq = frequency;
    snd_pcm_uframes_t buffer_size;

    buffer_size = this->_spec.samples;
    snd_pcm_hw_params_set_period_size_near(this->_pcm_handle,
        hardware_parameters, &buffer_size, NULL);
    if (buffer_size > static_cast<snd_pcm_uframes_t>(std::numeric_limits<int>::max()))
    {
        snd_pcm_close(this->_pcm_handle);
        return (ft_sound_error_platform_failure);
    }
    this->_spec.samples = static_cast<int>(buffer_size);
    error_code = snd_pcm_hw_params(this->_pcm_handle, hardware_parameters);
    if (error_code < 0)
    {
        snd_pcm_close(this->_pcm_handle);
        return (ft_sound_error_platform_failure);
    }
    this->_buffer_size = this->_spec.samples * this->_spec.channels * 2;
    this->_buffer = new uint8_t[this->_buffer_size];
    if (this->_buffer == NULL)
    {
        snd_pcm_close(this->_pcm_handle);
        return (ft_sound_error_out_of_memory);
    }
    this->_is_running = true;
    return (ft_sound_ok);
}

void ft_sound_device_alsa::close(void)
{
    this->stop();
    if (this->_is_running)
    {
        this->_is_running = false;
    }
    if (this->_pcm_handle != NULL)
    {
        snd_pcm_close(this->_pcm_handle);
        this->_pcm_handle = NULL;
    }
    if (this->_buffer != NULL)
    {
        delete[] this->_buffer;
        this->_buffer = NULL;
    }
    return ;
}

void ft_sound_device_alsa::pause(int pause_on)
{
    if (this->_pcm_handle == NULL)
    {
        return ;
    }
    snd_pcm_pause(this->_pcm_handle, pause_on);
    return ;
}

void ft_sound_device_alsa::play(const ft_sound_clip *clip)
{
    if (clip == NULL || this->_is_running == false)
    {
        return ;
    }
    this->stop();
    this->_current_clip = clip;
    this->_playback_pos = 0;
    pthread_create(&this->_playback_thread, NULL,
        ft_sound_device_alsa::sound_thread, this);
    return ;
}

void ft_sound_device_alsa::stop(void)
{
    if (this->_current_clip != NULL)
    {
        pthread_join(this->_playback_thread, NULL);
        this->_current_clip = NULL;
        this->_playback_pos = 0;
    }
    return ;
}

ft_sound_device *ft_create_sound_device_alsa(void)
{
    return (new ft_sound_device_alsa());
}

#endif
