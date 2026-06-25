#ifndef COMPATEBILITY_SOUND_DEVICE_WIN32_HPP
#define COMPATEBILITY_SOUND_DEVICE_WIN32_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
#if defined(_WIN32)

#include "../DUMB/sound_device.hpp"
#include "../DUMB/sound_clip.hpp"
#define WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#include <windows.h>

class ft_sound_device_win32 : public ft_sound_device
{
    private:
        HWAVEOUT                _wave_out;
        WAVEHDR                 _headers[2];
        uint8_t                 *_buffer;
        int32_t                 _buffer_size;
        ft_sound_spec           _spec;
        const ft_sound_clip     *_current_clip;
        ft_size_t               _playback_pos;

        static void CALLBACK wave_out_proc(HWAVEOUT wave_out,
            UINT message, DWORD_PTR instance, DWORD_PTR parameter_one,
            DWORD_PTR parameter_two);

    public:
        ft_sound_device_win32(void);
        ft_sound_device_win32(const ft_sound_device_win32 &other);
        ft_sound_device_win32(ft_sound_device_win32 &&other);
        ft_sound_device_win32 &operator=(const ft_sound_device_win32 &other) = delete;
        ft_sound_device_win32 &operator=(ft_sound_device_win32 &&other) = delete;
        ~ft_sound_device_win32(void);

        int32_t open(const ft_sound_spec *spec);
        void close(void);
        void pause(int32_t pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

#endif

#endif
