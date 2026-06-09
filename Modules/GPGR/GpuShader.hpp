#ifndef GPUR_SHADER_HPP
# define GPUR_SHADER_HPP

#include <cstdio>
#include "gl_funcs.hpp"
#include <cstdint>

class GpuShader
{
    GLuint _program;

    public:
        GpuShader() noexcept;
        GpuShader(const GpuShader &other) noexcept;
    ~GpuShader() noexcept;
    GpuShader &operator=(const GpuShader &other) noexcept;


        bool   compile(const char *vert_src, const char *frag_src) noexcept;
        void   destroy() noexcept;
        void   use() const noexcept;
        GLint  uniform(const char *name) const noexcept;
        bool   ready() const noexcept;
};

#endif
