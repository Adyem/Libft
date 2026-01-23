#if !defined(_WIN32) && !defined(__APPLE__)

#include "dumb_sound_internal.hpp"
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "dumb_sound_clip.hpp"
#include <unistd.h>
#include <string.h>
#include <limits>

ft_sound_device *ft_create_sound_device_alsa(void);


class ft_sound_device_alsa : public ft_sound_device
{
    private:
        snd_pcm_t           *_pcm_handle;
        pthread_t           _thread;
        bool                _is_running;
        ft_sound_spec       _spec;
        uint8_t             *_buffer;
        int                 _buffer_size;

        const ft_sound_clip *_current_clip;
        size_t              _playback_pos;
        pthread_t           _playback_thread;

        static void *sound_thread(void *arg)
        {
            ft_sound_device_alsa *self = static_cast<ft_sound_device_alsa *>(arg);

            while (self->_is_running && self->_current_clip != NULL && self->_playback_pos < self->_current_clip->get_size())
            {
                size_t to_copy = self->_current_clip->get_size() - self->_playback_pos;
                if (to_copy > static_cast<size_t>(self->_buffer_size))
                {
                    to_copy = self->_buffer_size;
                }
                memcpy(self->_buffer, self->_current_clip->get_data() + self->_playback_pos, to_copy);
                snd_pcm_writei(self->_pcm_handle, self->_buffer, to_copy / (self->_spec.channels * 2));
                self->_playback_pos += to_copy;
            }
            return (NULL);
        }

    public:
        ft_sound_device_alsa(void)
        {
            this->_pcm_handle = NULL;
            this->_is_running = false;
            this->_buffer = NULL;
            this->_buffer_size = 0;
            memset(&this->_spec, 0, sizeof(this->_spec));
            this->_current_clip = NULL;
            this->_playback_pos = 0;
            return ;
        }

        ~ft_sound_device_alsa(void)
        {
            this->close();
            return ;
        }

        int open(const ft_sound_spec *spec)
        {
            snd_pcm_hw_params_t *hw_params;
            int                 err;

            if (spec == NULL)
            {
                return (ft_sound_error_invalid_argument);
            }

            this->_spec = *spec;

            err = snd_pcm_open(&this->_pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
            if (err < 0)
            {
                return (ft_sound_error_platform_failure);
            }

            hw_params = NULL;
            snd_pcm_hw_params_alloca(&hw_params);
            snd_pcm_hw_params_any(this->_pcm_handle, hw_params);
            snd_pcm_hw_params_set_access(this->_pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
            snd_pcm_hw_params_set_format(this->_pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
            snd_pcm_hw_params_set_channels(this->_pcm_handle, hw_params, this->_spec.channels);
            unsigned int freq = this->_spec.freq;
            snd_pcm_hw_params_set_rate_near(this->_pcm_handle, hw_params, &freq, NULL);
            this->_spec.freq = freq;

            snd_pcm_uframes_t buffer_size = this->_spec.samples;
            snd_pcm_hw_params_set_period_size_near(this->_pcm_handle, hw_params, &buffer_size, NULL);
            if (buffer_size > static_cast<snd_pcm_uframes_t>(std::numeric_limits<int>::max()))
            {
                snd_pcm_close(this->_pcm_handle);
                return (ft_sound_error_platform_failure);
            }
            this->_spec.samples = static_cast<int>(buffer_size);
            
            err = snd_pcm_hw_params(this->_pcm_handle, hw_params);
            if (err < 0)
            {
                snd_pcm_close(this->_pcm_handle);
                return (ft_sound_error_platform_failure);
            }

            this->_buffer_size = this->_spec.samples * this->_spec.channels * 2;
            this->_buffer = new uint8_t[this->_buffer_size];
            if (this->_buffer == NULL)
            {
                snd_pcm_close(this->_pcm_handle);
                return (ft_sound_error_out_of_memory);
            }

            this->_is_running = true;

            return (ft_sound_ok);
        }

        void close(void)
        {
            this->stop();
            if (this->_is_running)
            {
                this->_is_running = false;
            }
            if (this->_pcm_handle != NULL)
            {
                snd_pcm_close(this->_pcm_handle);
                this->_pcm_handle = NULL;
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
            if (this->_pcm_handle == NULL)
            {
                return ;
            }
            snd_pcm_pause(this->_pcm_handle, pause_on);
            return ;
        }

        void play(const ft_sound_clip *clip)
        {
            if (clip == NULL || this->_is_running == false)
            {
                return ;
            }
            this->stop();
            this->_current_clip = clip;
            this->_playback_pos = 0;
            pthread_create(&this->_playback_thread, NULL, sound_thread, this);
        }

        void stop(void)
        {
            if (this->_current_clip != NULL)
            {
                pthread_join(this->_playback_thread, NULL);
                this->_current_clip = NULL;
                this->_playback_pos = 0;
            }
        }
};

ft_sound_device *ft_create_sound_device_alsa(void)
{
    return (new ft_sound_device_alsa());
}

#endif
