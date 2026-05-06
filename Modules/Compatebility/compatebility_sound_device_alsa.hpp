#ifndef COMPATEBILITY_SOUND_DEVICE_ALSA_HPP
#define COMPATEBILITY_SOUND_DEVICE_ALSA_HPP

#if !defined(_WIN32) && !defined(__APPLE__)

#include "../DUMB/sound_device.hpp"
#include "../DUMB/sound_clip.hpp"
#include <alsa/asoundlib.h>
#include <pthread.h>

class ft_sound_device_alsa : public ft_sound_device
{
    private:
        snd_pcm_t               *_pcm_handle;
        pthread_t               _thread;
        ft_bool                 _is_running;
        ft_sound_spec           _spec;
        uint8_t                 *_buffer;
        int32_t                 _buffer_size;
        const ft_sound_clip     *_current_clip;
        ft_size_t               _playback_pos;
        pthread_t               _playback_thread;

        static void *sound_thread(void *argument);

    public:
        ft_sound_device_alsa(void);
        ft_sound_device_alsa(const ft_sound_device_alsa &other);
        ft_sound_device_alsa(ft_sound_device_alsa &&other);
        ft_sound_device_alsa &operator=(const ft_sound_device_alsa &other) = delete;
        ft_sound_device_alsa &operator=(ft_sound_device_alsa &&other) = delete;
        ~ft_sound_device_alsa(void);

        int32_t open(const ft_sound_spec *spec);
        void close(void);
        void pause(int32_t pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

#endif

#endif
