#if defined(_WIN32)

#include "dumb_sound.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "dumb_sound_clip.hpp"
#include <string.h>


class ft_sound_device_win32 : public ft_sound_device
{
    private:
        HWAVEOUT        _wave_out;
        WAVEHDR         _headers[2];
        uint8_t         *_buffer;
        int             _buffer_size;
        ft_sound_spec   _spec;

        const ft_sound_clip *_current_clip;
        size_t              _playback_pos;

        static void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            ft_sound_device_win32 *self = (ft_sound_device_win32 *)dwInstance;

            if (uMsg == WOM_DONE)
            {
                if (self->_current_clip == NULL)
                {
                    return;
                }

                if (self->_playback_pos < self->_current_clip->get_size())
                {
                    WAVEHDR *header = (WAVEHDR *)dwParam1;
                    size_t to_copy = self->_current_clip->get_size() - self->_playback_pos;
                    if (to_copy > header->dwBufferLength)
                    {
                        to_copy = header->dwBufferLength;
                    }
                    memcpy(header->lpData, self->_current_clip->get_data() + self->_playback_pos, to_copy);
                    header->dwBufferLength = to_copy;
                    self->_playback_pos += to_copy;
                    waveOutWrite(hwo, header, sizeof(WAVEHDR));
                }
                else
                {
                    self->stop();
                }
            }
        }

    public:
        ft_sound_device_win32(void)
        {
            this->_wave_out = NULL;
            this->_buffer = NULL;
            this->_buffer_size = 0;
            memset(&this->_spec, 0, sizeof(this->_spec));
            this->_current_clip = NULL;
            this->_playback_pos = 0;
            return ;
        }

        ~ft_sound_device_win32(void)
        {
            this->close();
            return ;
        }

        int open(const ft_sound_spec *spec)
        {
            WAVEFORMATEX    wave_format;
            MMRESULT        result;

            if (spec == NULL)
            {
                return (ft_sound_error_invalid_argument);
            }

            this->_spec = *spec;
            this->_buffer_size = this->_spec.samples * this->_spec.channels * 2;

            wave_format.wFormatTag = WAVE_FORMAT_PCM;
            wave_format.nChannels = (WORD)this->_spec.channels;
            wave_format.nSamplesPerSec = (DWORD)this->_spec.freq;
            wave_format.nAvgBytesPerSec = (DWORD)(this->_spec.freq * this->_spec.channels * 2);
            wave_format.nBlockAlign = (WORD)(this->_spec.channels * 2);
            wave_format.wBitsPerSample = 16;
            wave_format.cbSize = 0;

            result = waveOutOpen(&this->_wave_out, WAVE_MAPPER, &wave_format, (DWORD_PTR)wave_out_proc, (DWORD_PTR)this, CALLBACK_FUNCTION);
            if (result != MMSYSERR_NOERROR)
            {
                return (ft_sound_error_platform_failure);
            }

            this->_buffer = new uint8_t[this->_buffer_size];
            if (this->_buffer == NULL)
            {
                waveOutClose(this->_wave_out);
                return (ft_sound_error_out_of_memory);
            }
            
            for (int i = 0; i < 2; i++)
            {
                memset(&this->_headers[i], 0, sizeof(WAVEHDR));
                this->_headers[i].dwBufferLength = this->_buffer_size / 2;
                this->_headers[i].lpData = (LPSTR)(this->_buffer + (i * (this->_buffer_size / 2)));
                waveOutPrepareHeader(this->_wave_out, &this->_headers[i], sizeof(WAVEHDR));
            }

            return (ft_sound_ok);
        }

        void close(void)
        {
            this->stop();
            if (this->_wave_out != NULL)
            {
                for (int i = 0; i < 2; i++)
                {
                    waveOutUnprepareHeader(this->_wave_out, &this->_headers[i], sizeof(WAVEHDR));
                }
                waveOutClose(this->_wave_out);
                this->_wave_out = NULL;
            }
            if (this->_buffer != NULL)
            {
                delete[] this->_buffer;
                this->_buffer = NULL;
            }
            return ;
        }

        void pause(int pause_on)
        {
            if (this->_wave_out == NULL)
            {
                return ;
            }
            if (pause_on)
            {
                waveOutPause(this->_wave_out);
            }
            else
            {
                waveOutRestart(this->_wave_out);
            }
            return ;
        }

        void play(const ft_sound_clip *clip)
        {
            if (clip == NULL || this->_wave_out == NULL)
            {
                return;
            }

            this->stop();
            this->_current_clip = clip;
            this->_playback_pos = 0;

            for (int i = 0; i < 2; i++)
            {
                wave_out_proc(this->_wave_out, WOM_DONE, (DWORD_PTR)this, (DWORD_PTR)&this->_headers[i], 0);
            }
        }

        void stop(void)
        {
            if (this->_current_clip != NULL)
            {
                this->_current_clip = NULL;
                this->_playback_pos = 0;
                waveOutReset(this->_wave_out);
            }
        }
};

ft_sound_device *ft_create_sound_device_win32(void)
{
    return (new ft_sound_device_win32());
}

#endif
