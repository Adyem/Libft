#include "dumb_sound_clip.hpp"
#include "dumb_io.hpp"
#include "../CMA/CMA.hpp"
#include "dumb_sound.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include <string.h>



ft_sound_clip::ft_sound_clip(void)
{
    this->_spec = new ft_sound_spec();
    // Initialize spec with some default values
    this->_spec->freq = 44100;
    this->_spec->channels = 2;
    this->_spec->samples = 4096;
    this->_spec->callback = NULL;
    this->_spec->userdata = NULL;
    return ;
}

ft_sound_clip::~ft_sound_clip(void)
{
    delete this->_spec;
    this->disable_thread_safety();
    return ;
}

int ft_sound_clip::prepare_thread_safety(void) noexcept
{
    if (this->_thread_safe_enabled && this->_mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_sound_clip::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    this->_thread_safe_enabled = false;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_sound_clip::load_wav(const char *file_path)
{
    char *buffer = NULL;
    size_t size = 0;
    if (ft_read_file(file_path, &buffer, &size) != ft_io_ok)
    {
        return (ft_sound_error_platform_failure);
    }

    if (size < 44)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    if (strncmp(buffer, "RIFF", 4) != 0 || strncmp(buffer + 8, "WAVE", 4) != 0)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    if (strncmp(buffer + 12, "fmt ", 4) != 0)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    uint32_t fmt_size = *reinterpret_cast<const uint32_t *>(buffer + 16);
    if (fmt_size < 16)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    this->_spec->channels = *reinterpret_cast<const uint16_t *>(buffer + 22);
    this->_spec->freq = *reinterpret_cast<const uint32_t *>(buffer + 24);

    char *data_chunk = strstr(buffer, "data");
    if (data_chunk == NULL)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    size_t data_offset = data_chunk - buffer;
    uint32_t data_size = *reinterpret_cast<const uint32_t *>(buffer + data_offset + 4);

    _data.assign(buffer + data_offset + 8, buffer + data_offset + 8 + data_size);

    cma_free(buffer);

    return (ft_sound_ok);
}

int ft_sound_clip::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void ft_sound_clip::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_sound_clip::is_thread_safe_enabled() const noexcept
{
    return (this->_thread_safe_enabled);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_sound_clip::runtime_mutex(void)
{
    return (this->_mutex);
}
#endif

const uint8_t *ft_sound_clip::get_data(void) const
{
    return (_data.data());
}

size_t ft_sound_clip::get_size(void) const
{
    return (_data.size());
}

const ft_sound_spec *ft_sound_clip::get_spec(void) const
{
    return (this->_spec);
}
