#ifndef COMPATEBILITY_SOUND_DEVICE_COREAUDIO_HPP
#define COMPATEBILITY_SOUND_DEVICE_COREAUDIO_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
#if defined(__APPLE__)

#include "../DUMB/sound_device.hpp"
#include "../DUMB/sound_clip.hpp"
#include <AudioToolbox/AudioToolbox.h>

class ft_sound_device_coreaudio : public ft_sound_device
{
    private:
        AudioQueueRef           _audio_queue;
        AudioQueueBufferRef     _buffers[2];
        ft_sound_spec           _spec;
        ft_bool                 _is_running;
        const ft_sound_clip     *_current_clip;
        ft_size_t               _playback_pos;

        static void audio_queue_callback(void *user_data,
            AudioQueueRef audio_queue, AudioQueueBufferRef audio_buffer);

    public:
        ft_sound_device_coreaudio(void);
        ft_sound_device_coreaudio(const ft_sound_device_coreaudio &other) = delete;
        ft_sound_device_coreaudio(ft_sound_device_coreaudio &&other) = delete;
        ft_sound_device_coreaudio &operator=(const ft_sound_device_coreaudio &other) = delete;
        ft_sound_device_coreaudio &operator=(ft_sound_device_coreaudio &&other) = delete;
        ~ft_sound_device_coreaudio(void);

        int32_t open(const ft_sound_spec *spec);
        void close(void);
        void pause(int32_t pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

#endif

#endif
