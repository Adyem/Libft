#ifndef DUMB_SOUND_LINUX_ALSA_HPP
# define DUMB_SOUND_LINUX_ALSA_HPP

#if !defined(_WIN32) && !defined(__APPLE__)

#include "dumb_sound.hpp"
#include <alsa/asoundlib.h>
#include <pthread.h>

class ft_sound_device_alsa : public ft_sound_device
{
    private:
        snd_pcm_t               *_pcm_handle;
        pthread_t               _thread;
        bool                    _is_running;
        ft_sound_spec           _spec;
        uint8_t                 *_buffer;
        int                     _buffer_size;
        const ft_sound_clip     *_current_clip;
        size_t                  _playback_pos;
        pthread_t               _playback_thread;

        static void *sound_thread(void *argument);

    public:
        ft_sound_device_alsa(void);
        ~ft_sound_device_alsa(void);

        int open(const ft_sound_spec *spec);
        void close(void);
        void pause(int pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

ft_sound_device *ft_create_sound_device_alsa(void);

#endif

#endif
