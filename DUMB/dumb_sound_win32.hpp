#ifndef DUMB_SOUND_WIN32_HPP
# define DUMB_SOUND_WIN32_HPP

#if defined(_WIN32)

#include "dumb_sound.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

class ft_sound_device_win32 : public ft_sound_device
{
    private:
        HWAVEOUT                _wave_out;
        WAVEHDR                 _headers[2];
        uint8_t                 *_buffer;
        int                     _buffer_size;
        ft_sound_spec           _spec;
        const ft_sound_clip     *_current_clip;
        size_t                  _playback_pos;

        static void CALLBACK wave_out_proc(HWAVEOUT wave_out,
                UINT message,
                DWORD_PTR instance,
                DWORD_PTR parameter_one,
                DWORD_PTR parameter_two);

    public:
        ft_sound_device_win32(void);
        ~ft_sound_device_win32(void);

        int open(const ft_sound_spec *spec);
        void close(void);
        void pause(int pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

ft_sound_device *ft_create_sound_device_win32(void);

#endif

#endif
