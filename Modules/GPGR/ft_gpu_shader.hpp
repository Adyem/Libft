#ifndef GPGR_FT_GPU_SHADER_HPP
# define GPGR_FT_GPU_SHADER_HPP

#include "../Basic/basic.hpp"
#include "gpgr_gl_funcs.hpp"
#include <cstdint>

class ft_gpu_shader
{
    GLuint _program;
    uint8_t _initialised_state;

    public:
        ft_gpu_shader() noexcept;
        ft_gpu_shader(const ft_gpu_shader &other) = delete;
        ft_gpu_shader(ft_gpu_shader &&other) = delete;
        ft_gpu_shader &operator=(const ft_gpu_shader &other) = delete;
        ft_gpu_shader &operator=(ft_gpu_shader &&other) = delete;
        ~ft_gpu_shader() noexcept;

        int32_t initialize(const char *vert_src, const char *frag_src) noexcept;
        int32_t destroy() noexcept;
        int32_t move(ft_gpu_shader &other) noexcept;
        void   use() const noexcept;
        int32_t uniform(const char *name) const noexcept;
        ft_bool ready() const noexcept;
};

#endif
