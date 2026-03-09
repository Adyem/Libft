#if defined(_WIN32)

#include "compatebility_internal.hpp"
#include "compatebility_sound_device_win32.hpp"
#include <mmsystem.h>
#include <new>
#include <string.h>
#include <windows.h>

void CALLBACK ft_sound_device_win32::wave_out_proc(HWAVEOUT wave_out,
        UINT message, DWORD_PTR instance, DWORD_PTR parameter_one,
        DWORD_PTR parameter_two)
{
    (void)parameter_two;
    ft_sound_device_win32 *sound_device;

    sound_device = reinterpret_cast<ft_sound_device_win32 *>(instance);
    if (message == WOM_DONE)
    {
        if (sound_device->_current_clip == ft_nullptr)
            return ;
        if (sound_device->_playback_pos < sound_device->_current_clip->get_size())
        {
            WAVEHDR *header;
            ft_size_t bytes_to_copy;

            header = reinterpret_cast<WAVEHDR *>(parameter_one);
            bytes_to_copy = sound_device->_current_clip->get_size()
                - sound_device->_playback_pos;
            if (bytes_to_copy > header->dwBufferLength)
                bytes_to_copy = header->dwBufferLength;
            memcpy(header->lpData,
                sound_device->_current_clip->get_data() + sound_device->_playback_pos,
                static_cast<ft_size_t>(bytes_to_copy));
            header->dwBufferLength = static_cast<DWORD>(bytes_to_copy);
            sound_device->_playback_pos += bytes_to_copy;
            waveOutWrite(wave_out, header, sizeof(WAVEHDR));
        }
        else
            sound_device->stop();
    }
    return ;
}

ft_sound_device_win32::ft_sound_device_win32(void)
{
    this->_wave_out = ft_nullptr;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_win32::ft_sound_device_win32(const ft_sound_device_win32 &other)
    : ft_sound_device(other)
{
    (void)other;
    this->_wave_out = ft_nullptr;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_win32::ft_sound_device_win32(ft_sound_device_win32 &&other)
    : ft_sound_device(static_cast<ft_sound_device &&>(other))
{
    (void)other;
    this->_wave_out = ft_nullptr;
    this->_buffer = ft_nullptr;
    this->_buffer_size = 0;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_win32::~ft_sound_device_win32(void)
{
    this->close();
    return ;
}

int32_t ft_sound_device_win32::open(const ft_sound_spec *spec)
{
    WAVEFORMATEX wave_format;
    MMRESULT result;
    int32_t buffer_index;

    if (spec == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->_spec = *spec;
    this->_buffer_size = this->_spec.samples * this->_spec.channels * 2;
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = static_cast<WORD>(this->_spec.channels);
    wave_format.nSamplesPerSec = static_cast<DWORD>(this->_spec.freq);
    wave_format.nAvgBytesPerSec = static_cast<DWORD>(this->_spec.freq
            * this->_spec.channels * 2);
    wave_format.nBlockAlign = static_cast<WORD>(this->_spec.channels * 2);
    wave_format.wBitsPerSample = 16;
    wave_format.cbSize = 0;
    result = waveOutOpen(&this->_wave_out, WAVE_MAPPER, &wave_format,
        static_cast<DWORD_PTR>(ft_sound_device_win32::wave_out_proc),
        static_cast<DWORD_PTR>(reinterpret_cast<uintptr_t>(this)),
        CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
        return (FT_ERR_IO);
    this->_buffer = new (std::nothrow) uint8_t[this->_buffer_size];
    if (this->_buffer == ft_nullptr)
    {
        waveOutClose(this->_wave_out);
        return (FT_ERR_NO_MEMORY);
    }
    buffer_index = 0;
    while (buffer_index < 2)
    {
        memset(&this->_headers[buffer_index], 0, sizeof(WAVEHDR));
        this->_headers[buffer_index].dwBufferLength = this->_buffer_size / 2;
        this->_headers[buffer_index].lpData = reinterpret_cast<LPSTR>(
            this->_buffer + (buffer_index * (this->_buffer_size / 2)));
        waveOutPrepareHeader(this->_wave_out, &this->_headers[buffer_index],
            sizeof(WAVEHDR));
        buffer_index++;
    }
    return (FT_ERR_SUCCESS);
}

void ft_sound_device_win32::close(void)
{
    this->stop();
    if (this->_wave_out != ft_nullptr)
    {
        int32_t buffer_index;

        buffer_index = 0;
        while (buffer_index < 2)
        {
            waveOutUnprepareHeader(this->_wave_out, &this->_headers[buffer_index],
                sizeof(WAVEHDR));
            buffer_index++;
        }
        waveOutClose(this->_wave_out);
        this->_wave_out = ft_nullptr;
    }
    if (this->_buffer != ft_nullptr)
    {
        delete[] this->_buffer;
        this->_buffer = ft_nullptr;
    }
    return ;
}

void ft_sound_device_win32::pause(int32_t pause_on)
{
    if (this->_wave_out == ft_nullptr)
        return ;
    if (pause_on)
        waveOutPause(this->_wave_out);
    else
        waveOutRestart(this->_wave_out);
    return ;
}

void ft_sound_device_win32::play(const ft_sound_clip *clip)
{
    int32_t buffer_index;

    if (clip == ft_nullptr || this->_wave_out == ft_nullptr)
        return ;
    this->stop();
    this->_current_clip = clip;
    this->_playback_pos = 0;
    buffer_index = 0;
    while (buffer_index < 2)
    {
        ft_sound_device_win32::wave_out_proc(this->_wave_out, WOM_DONE,
            static_cast<DWORD_PTR>(reinterpret_cast<uintptr_t>(this)),
            static_cast<DWORD_PTR>(reinterpret_cast<uintptr_t>(&this->_headers[buffer_index])),
            0);
        buffer_index++;
    }
    return ;
}

void ft_sound_device_win32::stop(void)
{
    if (this->_current_clip != ft_nullptr)
    {
        this->_current_clip = ft_nullptr;
        this->_playback_pos = 0;
        waveOutReset(this->_wave_out);
    }
    return ;
}

ft_sound_device *cmp_create_sound_device(void)
{
    return (new ft_sound_device_win32());
}

#endif
