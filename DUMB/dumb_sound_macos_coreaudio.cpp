#if defined(__APPLE__)

#include "dumb_sound.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <string.h>

class ft_sound_device_coreaudio : public ft_sound_device
{
    private:
        AudioQueueRef           _audio_queue;
        AudioQueueBufferRef     _buffers[2];
        ft_sound_spec           _spec;
        bool                    _is_running;

        const ft_sound_clip     *_current_clip;
        size_t                  _playback_pos;

        static void audio_queue_callback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
        {
            ft_sound_device_coreaudio *self = (ft_sound_device_coreaudio *)inUserData;

            if (self->_current_clip == NULL)
            {
                return;
            }

            if (self->_playback_pos < self->_current_clip->get_size())
            {
                size_t to_copy = self->_current_clip->get_size() - self->_playback_pos;
                if (to_copy > inBuffer->mAudioDataBytesCapacity)
                {
                    to_copy = inBuffer->mAudioDataBytesCapacity;
                }
                memcpy(inBuffer->mAudioData, self->_current_clip->get_data() + self->_playback_pos, to_copy);
                inBuffer->mAudioDataByteSize = to_copy;
                self->_playback_pos += to_copy;
                AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
            }
            else
            {
                self->stop();
            }
        }

    public:
        ft_sound_device_coreaudio(void)
        {
            this->_audio_queue = NULL;
            this->_is_running = false;
            memset(&this->_spec, 0, sizeof(this->_spec));
            this->_current_clip = NULL;
            this->_playback_pos = 0;
            return ;
        }

        ~ft_sound_device_coreaudio(void)
        {
            this->close();
            return ;
        }

        int open(const ft_sound_spec *spec)
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

            status = AudioQueueNewOutput(&format, audio_queue_callback, this, NULL, NULL, 0, &this->_audio_queue);
            if (status != noErr)
            {
                return (ft_sound_error_platform_failure);
            }

            for (int i = 0; i < 2; ++i)
            {
                status = AudioQueueAllocateBuffer(this->_audio_queue, 4096, &this->_buffers[i]);
                if (status != noErr)
                {
                    AudioQueueDispose(this->_audio_queue, true);
                    return (ft_sound_error_out_of_memory);
                }
            }

            return (ft_sound_ok);
        }

        void close(void)
        {
            this->stop();
            if (this->_audio_queue != NULL)
            {
                AudioQueueDispose(this->_audio_queue, true);
                this->_audio_queue = NULL;
            }
            return ;
        }

        void pause(int pause_on)
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

        void play(const ft_sound_clip *clip)
        {
            if (clip == NULL || this->_audio_queue == NULL)
            {
                return;
            }

            this->stop();
            this->_current_clip = clip;
            this->_playback_pos = 0;

            for (int i = 0; i < 2; ++i)
            {
                audio_queue_callback(this, this->_audio_queue, this->_buffers[i]);
            }

            AudioQueueStart(this->_audio_queue, NULL);
            this->_is_running = true;
        }

        void stop(void)
        {
            if (this->_is_running)
            {
                AudioQueueStop(this->_audio_queue, true);
                this->_is_running = false;
                this->_current_clip = NULL;
                this->_playback_pos = 0;
            }
        }
};

ft_sound_device *ft_create_sound_device_coreaudio(void)
{
    return (new ft_sound_device_coreaudio());
}

#endif
