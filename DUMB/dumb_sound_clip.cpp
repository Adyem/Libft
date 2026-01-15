#include "dumb_sound_clip.hpp"
#include "dumb_io.hpp"
#include "CMA/CMA.hpp"
#include "dumb_sound.hpp"
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

    uint32_t fmt_size = *(uint32_t *)(buffer + 16);
    if (fmt_size < 16)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    this->_spec->channels = *(uint16_t *)(buffer + 22);
    this->_spec->freq = *(uint32_t *)(buffer + 24);

    char *data_chunk = strstr(buffer, "data");
    if (data_chunk == NULL)
    {
        cma_free(buffer);
        return (ft_sound_error_invalid_argument);
    }

    size_t data_offset = data_chunk - buffer;
    uint32_t data_size = *(uint32_t *)(buffer + data_offset + 4);

    _data.assign(buffer + data_offset + 8, buffer + data_offset + 8 + data_size);

    cma_free(buffer);

    return (ft_sound_ok);
}

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
