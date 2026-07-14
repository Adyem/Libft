#include "gpgr_gl_funcs.hpp"


#if !defined(__APPLE__)

PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays_ptr         = nullptr;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray_ptr         = nullptr;
PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays_ptr      = nullptr;
PFNGLGENBUFFERSPROC              glGenBuffers_ptr              = nullptr;
PFNGLBINDBUFFERPROC              glBindBuffer_ptr              = nullptr;
PFNGLBUFFERDATAPROC              glBufferData_ptr              = nullptr;
PFNGLBUFFERSUBDATAPROC           glBufferSubData_ptr           = nullptr;
PFNGLDELETEBUFFERSPROC           glDeleteBuffers_ptr           = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC    glVertexAttribIPointer_ptr    = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer_ptr     = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = nullptr;
PFNGLCREATESHADERPROC            glCreateShader_ptr            = nullptr;
PFNGLSHADERSOURCEPROC            glShaderSource_ptr            = nullptr;
PFNGLCOMPILESHADERPROC           glCompileShader_ptr           = nullptr;
PFNGLGETSHADERIVPROC             glGetShaderiv_ptr             = nullptr;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog_ptr        = nullptr;
PFNGLDELETESHADERPROC            glDeleteShader_ptr            = nullptr;
PFNGLCREATEPROGRAMPROC           glCreateProgram_ptr           = nullptr;
PFNGLATTACHSHADERPROC            glAttachShader_ptr            = nullptr;
PFNGLLINKPROGRAMPROC             glLinkProgram_ptr             = nullptr;
PFNGLGETPROGRAMIVPROC            glGetProgramiv_ptr            = nullptr;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog_ptr       = nullptr;
PFNGLUSEPROGRAMPROC              glUseProgram_ptr              = nullptr;
PFNGLDELETEPROGRAMPROC           glDeleteProgram_ptr           = nullptr;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation_ptr      = nullptr;
PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv_ptr        = nullptr;
PFNGLUNIFORM1IPROC               glUniform1i_ptr               = nullptr;
PFNGLUNIFORM3FVPROC              glUniform3fv_ptr              = nullptr;
PFNGLUNIFORM4FVPROC              glUniform4fv_ptr              = nullptr;
PFNGLUNIFORM4FPROC               glUniform4f_ptr               = nullptr;
PFNGLUNIFORM2FPROC               glUniform2f_ptr               = nullptr;
PFNGLUNIFORM1FPROC               glUniform1f_ptr               = nullptr;
PFNGLGENERATEMIPMAPPROC          glGenerateMipmap_ptr          = nullptr;
PFNGLACTIVETEXTUREPROC           glActiveTexture_ptr           = nullptr;

static void gpgr_clear_gl_functions() noexcept
{
    glGenVertexArrays_ptr = nullptr;
    glBindVertexArray_ptr = nullptr;
    glDeleteVertexArrays_ptr = nullptr;
    glGenBuffers_ptr = nullptr;
    glBindBuffer_ptr = nullptr;
    glBufferData_ptr = nullptr;
    glBufferSubData_ptr = nullptr;
    glDeleteBuffers_ptr = nullptr;
    glVertexAttribIPointer_ptr = nullptr;
    glVertexAttribPointer_ptr = nullptr;
    glEnableVertexAttribArray_ptr = nullptr;
    glCreateShader_ptr = nullptr;
    glShaderSource_ptr = nullptr;
    glCompileShader_ptr = nullptr;
    glGetShaderiv_ptr = nullptr;
    glGetShaderInfoLog_ptr = nullptr;
    glDeleteShader_ptr = nullptr;
    glCreateProgram_ptr = nullptr;
    glAttachShader_ptr = nullptr;
    glLinkProgram_ptr = nullptr;
    glGetProgramiv_ptr = nullptr;
    glGetProgramInfoLog_ptr = nullptr;
    glUseProgram_ptr = nullptr;
    glDeleteProgram_ptr = nullptr;
    glGetUniformLocation_ptr = nullptr;
    glUniformMatrix4fv_ptr = nullptr;
    glUniform1i_ptr = nullptr;
    glUniform3fv_ptr = nullptr;
    glUniform4fv_ptr = nullptr;
    glUniform4f_ptr = nullptr;
    glUniform2f_ptr = nullptr;
    glUniform1f_ptr = nullptr;
    glGenerateMipmap_ptr = nullptr;
    glActiveTexture_ptr = nullptr;
    return ;
}

#if defined(_WIN32)
static void *gpgr_get_proc_address(const char *name)
{
    PROC proc = wglGetProcAddress(name);
    if (proc != nullptr)
        return reinterpret_cast<void *>(proc);

    HMODULE module = GetModuleHandleA("opengl32.dll");
    if (module == nullptr)
        return nullptr;
    return reinterpret_cast<void *>(GetProcAddress(module, name));
}
#else
static void *gpgr_get_proc_address(const char *name)
{
    return reinterpret_cast<void *>(glXGetProcAddressARB(
        reinterpret_cast<const GLubyte *>(name)));
}
#endif

template <typename Fn>
static ft_bool gpgr_load_symbol(Fn &symbol, const char *name)
{
    void *proc = gpgr_get_proc_address(name);
    if (proc == nullptr)
        return (FT_FALSE);
    ft_memcpy(&symbol, &proc, sizeof(symbol));
    return (FT_TRUE);
}

#define GPGR_LOAD(name) \
    if (gpgr_load_symbol(name, #name) == FT_FALSE) return (FT_FALSE)

ft_bool gpgr_load_gl_functions() noexcept
{
    gpgr_clear_gl_functions();
    GPGR_LOAD(glGenVertexArrays);
    GPGR_LOAD(glBindVertexArray);
    GPGR_LOAD(glDeleteVertexArrays);
    GPGR_LOAD(glGenBuffers);
    GPGR_LOAD(glBindBuffer);
    GPGR_LOAD(glBufferData);
    GPGR_LOAD(glDeleteBuffers);
    GPGR_LOAD(glVertexAttribIPointer);
    GPGR_LOAD(glEnableVertexAttribArray);
    GPGR_LOAD(glCreateShader);
    GPGR_LOAD(glShaderSource);
    GPGR_LOAD(glCompileShader);
    GPGR_LOAD(glGetShaderiv);
    GPGR_LOAD(glGetShaderInfoLog);
    GPGR_LOAD(glDeleteShader);
    GPGR_LOAD(glCreateProgram);
    GPGR_LOAD(glAttachShader);
    GPGR_LOAD(glLinkProgram);
    GPGR_LOAD(glGetProgramiv);
    GPGR_LOAD(glGetProgramInfoLog);
    GPGR_LOAD(glUseProgram);
    GPGR_LOAD(glDeleteProgram);
    GPGR_LOAD(glGetUniformLocation);
    GPGR_LOAD(glUniformMatrix4fv);
    GPGR_LOAD(glUniform1i);
    GPGR_LOAD(glUniform3fv);
    GPGR_LOAD(glUniform4fv);
    GPGR_LOAD(glUniform4f);
    GPGR_LOAD(glUniform2f);
    GPGR_LOAD(glUniform1f);
    GPGR_LOAD(glGenerateMipmap);
    {
        void *proc = gpgr_get_proc_address("glBufferSubData");
        if (proc == nullptr)
            return (FT_FALSE);
        ft_memcpy(&glBufferSubData_ptr, &proc, sizeof(proc));
    }
    {
        void *proc = gpgr_get_proc_address("glVertexAttribPointer");
        if (proc == nullptr)
            return (FT_FALSE);
        ft_memcpy(&glVertexAttribPointer_ptr, &proc, sizeof(proc));
    }
    GPGR_LOAD(glActiveTexture);
    return (FT_TRUE);
}

#undef GPGR_LOAD

#endif
