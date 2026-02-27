#ifndef DUMB_SOUND_MACOS_COREAUDIO_HPP
# define DUMB_SOUND_MACOS_COREAUDIO_HPP

#if defined(__APPLE__)

#include "dumb_sound.hpp"
#include <AudioToolbox/AudioToolbox.h>

class ft_sound_device_coreaudio : public ft_sound_device
{
    private:
        AudioQueueRef           _audio_queue;
        AudioQueueBufferRef     _buffers[2];
        ft_sound_spec           _spec;
        bool                    _is_running;
        const ft_sound_clip     *_current_clip;
        size_t                  _playback_pos;

        static void audio_queue_callback(void *user_data,
                AudioQueueRef audio_queue,
                AudioQueueBufferRef audio_buffer);

    public:
        ft_sound_device_coreaudio(void);
        ~ft_sound_device_coreaudio(void);

        int open(const ft_sound_spec *spec);
        void close(void);
        void pause(int pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

ft_sound_device *ft_create_sound_device_coreaudio(void);

#endif

#endif
