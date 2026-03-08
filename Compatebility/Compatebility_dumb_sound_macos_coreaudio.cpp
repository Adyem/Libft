#if defined(__APPLE__)

#include "compatebility_internal.hpp"
#include "compatebility_sound_device_coreaudio.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <string.h>

void ft_sound_device_coreaudio::audio_queue_callback(void *user_data,
        AudioQueueRef audio_queue, AudioQueueBufferRef audio_buffer)
{
    ft_sound_device_coreaudio *sound_device;

    sound_device = static_cast<ft_sound_device_coreaudio *>(user_data);
    if (sound_device->_current_clip == ft_nullptr)
        return ;
    if (sound_device->_playback_pos < sound_device->_current_clip->get_size())
    {
        ft_size_t bytes_to_copy;

        bytes_to_copy = sound_device->_current_clip->get_size()
            - sound_device->_playback_pos;
        if (bytes_to_copy > static_cast<ft_size_t>(audio_buffer->mAudioDataBytesCapacity))
            bytes_to_copy = static_cast<ft_size_t>(audio_buffer->mAudioDataBytesCapacity);
        memcpy(audio_buffer->mAudioData,
            sound_device->_current_clip->get_data() + sound_device->_playback_pos,
            static_cast<size_t>(bytes_to_copy));
        audio_buffer->mAudioDataByteSize = static_cast<uint32_t>(bytes_to_copy);
        sound_device->_playback_pos += bytes_to_copy;
        AudioQueueEnqueueBuffer(audio_queue, audio_buffer, 0, ft_nullptr);
    }
    else
        sound_device->stop();
    return ;
}

ft_sound_device_coreaudio::ft_sound_device_coreaudio(void)
{
    this->_audio_queue = ft_nullptr;
    this->_is_running = FT_FALSE;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_coreaudio::ft_sound_device_coreaudio(
    const ft_sound_device_coreaudio &other)
    : ft_sound_device(other)
{
    (void)other;
    this->_audio_queue = ft_nullptr;
    this->_is_running = FT_FALSE;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_coreaudio::ft_sound_device_coreaudio(
    ft_sound_device_coreaudio &&other)
    : ft_sound_device(static_cast<ft_sound_device &&>(other))
{
    (void)other;
    this->_audio_queue = ft_nullptr;
    this->_is_running = FT_FALSE;
    memset(&this->_spec, 0, sizeof(this->_spec));
    this->_current_clip = ft_nullptr;
    this->_playback_pos = 0;
    return ;
}

ft_sound_device_coreaudio::~ft_sound_device_coreaudio(void)
{
    this->close();
    return ;
}

int32_t ft_sound_device_coreaudio::open(const ft_sound_spec *spec)
{
    AudioStreamBasicDescription format;
    OSStatus status;
    int32_t buffer_index;

    if (spec == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
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
        this, ft_nullptr, ft_nullptr, 0, &this->_audio_queue);
    if (status != noErr)
        return (FT_ERR_IO);
    buffer_index = 0;
    while (buffer_index < 2)
    {
        status = AudioQueueAllocateBuffer(this->_audio_queue, 4096,
            &this->_buffers[buffer_index]);
        if (status != noErr)
        {
            AudioQueueDispose(this->_audio_queue, FT_TRUE);
            return (FT_ERR_NO_MEMORY);
        }
        buffer_index++;
    }
    return (FT_ERR_SUCCESS);
}

void ft_sound_device_coreaudio::close(void)
{
    this->stop();
    if (this->_audio_queue != ft_nullptr)
    {
        AudioQueueDispose(this->_audio_queue, FT_TRUE);
        this->_audio_queue = ft_nullptr;
    }
    return ;
}

void ft_sound_device_coreaudio::pause(int32_t pause_on)
{
    if (this->_audio_queue == ft_nullptr)
        return ;
    if (pause_on)
    {
        if (this->_is_running == FT_TRUE)
        {
            AudioQueuePause(this->_audio_queue);
            this->_is_running = FT_FALSE;
        }
    }
    else if (this->_is_running == FT_FALSE)
    {
        AudioQueueStart(this->_audio_queue, ft_nullptr);
        this->_is_running = FT_TRUE;
    }
    return ;
}

void ft_sound_device_coreaudio::play(const ft_sound_clip *clip)
{
    int32_t buffer_index;

    if (clip == ft_nullptr || this->_audio_queue == ft_nullptr)
        return ;
    this->stop();
    this->_current_clip = clip;
    this->_playback_pos = 0;
    buffer_index = 0;
    while (buffer_index < 2)
    {
        ft_sound_device_coreaudio::audio_queue_callback(this, this->_audio_queue,
            this->_buffers[buffer_index]);
        buffer_index++;
    }
    AudioQueueStart(this->_audio_queue, ft_nullptr);
    this->_is_running = FT_TRUE;
    return ;
}

void ft_sound_device_coreaudio::stop(void)
{
    if (this->_is_running == FT_TRUE)
    {
        AudioQueueStop(this->_audio_queue, FT_TRUE);
        this->_is_running = FT_FALSE;
        this->_current_clip = ft_nullptr;
        this->_playback_pos = 0;
    }
    return ;
}

ft_sound_device *cmp_create_sound_device(void)
{
    return (new ft_sound_device_coreaudio());
}

#endif
