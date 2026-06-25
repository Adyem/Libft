#include "ft_gpu_shader.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Printf/printf.hpp"

ft_gpu_shader::ft_gpu_shader() noexcept
    : _program(0), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_gpu_shader::~ft_gpu_shader() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    this->_program = 0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

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
        ft_fprintf(stderr, "[gpgr] shader compile error: %s\n", log);
        glDeleteShader(shader);
        return (0);
    }
    return (shader);
}

int32_t ft_gpu_shader::initialize(const char *vert_src, const char *frag_src) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "ft_gpu_shader::initialize", "object is already initialised");
    }
    if (vert_src == nullptr || frag_src == nullptr)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INVALID_ARGUMENT);
    }

    GLuint vert = compile_stage(GL_VERTEX_SHADER, vert_src);
    if (vert == 0)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    GLuint frag = compile_stage(GL_FRAGMENT_SHADER, frag_src);
    if (frag == 0)
    {
        glDeleteShader(vert);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    GLuint program = glCreateProgram();
    if (program == 0)
    {
        glDeleteShader(vert);
        glDeleteShader(frag);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_NO_MEMORY);
    }
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok == 0)
    {
        char log[512];
        glGetProgramInfoLog(program, static_cast<GLsizei>(sizeof(log)),
            nullptr, log);
        ft_fprintf(stderr, "[gpgr] shader link error: %s\n", log);
        glDeleteProgram(program);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    if (this->_program != 0)
        glDeleteProgram(this->_program);
    this->_program = program;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_gpu_shader::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_program = 0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_program != 0)
    {
        glDeleteProgram(this->_program);
        this->_program = 0;
    }
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_gpu_shader::move(ft_gpu_shader &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_gpu_shader::move", "source object is uninitialised");
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_program = 0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_program = other._program;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._program = 0;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

void ft_gpu_shader::use() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_gpu_shader::use");
    glUseProgram(this->_program);
    return ;
}

int32_t ft_gpu_shader::uniform(const char *name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_gpu_shader::uniform");
    return (glGetUniformLocation(this->_program, name));
}

ft_bool ft_gpu_shader::ready() const noexcept
{
    return (this->_initialised_state == FT_CLASS_STATE_INITIALISED
        && this->_program != 0);
}
