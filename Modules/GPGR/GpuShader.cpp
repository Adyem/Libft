#include "GpuShader.hpp"

GpuShader::GpuShader() noexcept : _program(0) {}

GpuShader::GpuShader(const GpuShader &other) noexcept { (void)other; }

GpuShader::~GpuShader() noexcept { destroy(); }

GpuShader &GpuShader::operator=(const GpuShader &other) noexcept
{ (void)other; return *this; }

static GLuint compile_stage(GLenum type, const char *src) noexcept
{
    GLuint shader = glCreateShader(type);
    if (shader == 0)
        return (0);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0)
    {
        char log[512];
        glGetShaderInfoLog(shader, static_cast<GLsizei>(sizeof(log)),
            nullptr, log);
        std::fprintf(stderr, "[gpur] shader compile error: %s\n", log);
        glDeleteShader(shader);
        return (0);
    }
    return (shader);
}


bool GpuShader::compile(const char *vert_src, const char *frag_src) noexcept
{
    destroy();
    GLuint vert = compile_stage(GL_VERTEX_SHADER,   vert_src);
    if (vert == 0)
        return (false);
    GLuint frag = compile_stage(GL_FRAGMENT_SHADER, frag_src);
    if (frag == 0)
    {
        glDeleteShader(vert);
        return (false);
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok == 0)
    {
        char log[512];
        glGetProgramInfoLog(prog, static_cast<GLsizei>(sizeof(log)),
            nullptr, log);
        std::fprintf(stderr, "[gpur] shader link error: %s\n", log);
        glDeleteProgram(prog);
        return (false);
    }
    _program = prog;
    return (true);
}

void GpuShader::destroy() noexcept
{
    if (_program != 0)
    {
        glDeleteProgram(_program);
        _program = 0;
    }
}

void GpuShader::use() const noexcept
{
    glUseProgram(_program);
}

GLint GpuShader::uniform(const char *name) const noexcept
{
    return (glGetUniformLocation(_program, name));
}

bool GpuShader::ready() const noexcept { return _program != 0; }
