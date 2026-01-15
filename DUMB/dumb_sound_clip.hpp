#pragma once

#include <vector>
#include <stdint.h>

struct ft_sound_spec;

class ft_sound_clip
{
    private:
        std::vector<uint8_t> _data;
        ft_sound_spec *_spec;

    public:
        ft_sound_clip(void);
        ~ft_sound_clip(void);

        int load_wav(const char *file_path);
        const uint8_t *get_data(void) const;
        size_t get_size(void) const;
        const ft_sound_spec *get_spec(void) const;
};
