#if defined(__APPLE__)

#include "dumb_sound_macos_coreaudio.hpp"
#include <string.h>

void ft_sound_device_coreaudio::audio_queue_callback(void *user_data,
        AudioQueueRef audio_queue,
        AudioQueueBufferRef audio_buffer)
{
    ft_sound_device_coreaudio *sound_device;

    sound_device = static_cast<ft_sound_device_coreaudio *>(user_data);
    if (sound_device->_current_clip == NULL)
    {
        return ;
    }
    if (sound_device->_playback_pos < sound_device->_current_clip->get_size())
    {
        size_t bytes_to_copy;

        bytes_to_copy = sound_device->_current_clip->get_size()
            - sound_device->_playback_pos;
        if (bytes_to_copy > audio_buffer->mAudioDataBytesCapacity)
        {
            bytes_to_copy = audio_buffer->mAudioDataBytesCapacity;
        }
        memcpy(audio_buffer->mAudioData,
            sound_device->_current_clip->get_data() + sound_device->_playback_pos,
            bytes_to_copy);
        audio_buffer->mAudioDataByteSize = bytes_to_copy;
        sound_device->_playback_pos += bytes_to_copy;
        AudioQueueEnqueueBuffer(audio_queue, audio_buffer, 0, NULL);
    }
    else
    {
        sound_device->stop();
    }
    return ;
}

ft_sound_device_coreaudio::ft_sound_device_coreaudio(void)
{
    this->_audio_queue = NULL;
    this->_is_running = false;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = NULL;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_coreaudio::~ft_sound_device_coreaudio(void)
{
    this->close();
    return ;
}

int ft_sound_device_coreaudio::open(const ft_sound_spec *spec)
{
    AudioStreamBasicDescription format;
    OSStatus                    status;

    if (spec == NULL)
    {
        return (ft_sound_error_invalid_argument);
    }
    this->_spec = *spec;
    format.mSampleRate = this->_spec.freq;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = this->_spec.channels;
    format.mBitsPerChannel = 16;
    format.mBytesPerPacket = this->_spec.channels * 2;
    format.mBytesPerFrame = this->_spec.channels * 2;
    format.mReserved = 0;
    status = AudioQueueNewOutput(&format, ft_sound_device_coreaudio::audio_queue_callback,
        this, NULL, NULL, 0, &this->_audio_queue);
    if (status != noErr)
    {
        return (ft_sound_error_platform_failure);
    }
    int buffer_index;

    buffer_index = 0;
    while (buffer_index < 2)
    {
        status = AudioQueueAllocateBuffer(this->_audio_queue, 4096,
            &this->_buffers[buffer_index]);
        if (status != noErr)
        {
            AudioQueueDispose(this->_audio_queue, true);
            return (ft_sound_error_out_of_memory);
        }
        buffer_index++;
    }
    return (ft_sound_ok);
}

void ft_sound_device_coreaudio::close(void)
{
    this->stop();
    if (this->_audio_queue != NULL)
    {
        AudioQueueDispose(this->_audio_queue, true);
        this->_audio_queue = NULL;
    }
    return ;
}

void ft_sound_device_coreaudio::pause(int pause_on)
{
    if (this->_audio_queue == NULL)
    {
        return ;
    }
    if (pause_on)
    {
        if (this->_is_running)
        {
            AudioQueuePause(this->_audio_queue);
            this->_is_running = false;
        }
    }
    else
    {
        if (!this->_is_running)
        {
            AudioQueueStart(this->_audio_queue, NULL);
            this->_is_running = true;
        }
    }
    return ;
}

void ft_sound_device_coreaudio::play(const ft_sound_clip *clip)
{
    if (clip == NULL || this->_audio_queue == NULL)
    {
        return ;
    }
    this->stop();
    this->_current_clip = clip;
    this->_playback_pos = 0;
    int buffer_index;

    buffer_index = 0;
    while (buffer_index < 2)
    {
        ft_sound_device_coreaudio::audio_queue_callback(this, this->_audio_queue,
            this->_buffers[buffer_index]);
        buffer_index++;
    }
    AudioQueueStart(this->_audio_queue, NULL);
    this->_is_running = true;
    return ;
}

void ft_sound_device_coreaudio::stop(void)
{
    if (this->_is_running)
    {
        AudioQueueStop(this->_audio_queue, true);
        this->_is_running = false;
        this->_current_clip = NULL;
        this->_playback_pos = 0;
    }
    return ;
}

ft_sound_device *ft_create_sound_device_coreaudio(void)
{
    return (new ft_sound_device_coreaudio());
}

#endif
