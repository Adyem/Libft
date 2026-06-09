#include "gl_funcs.hpp"


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
PFNGLUNIFORM2FPROC               glUniform2f_ptr               = nullptr;
PFNGLUNIFORM1FPROC               glUniform1f_ptr               = nullptr;
PFNGLGENERATEMIPMAPPROC          glGenerateMipmap_ptr          = nullptr;
PFNGLACTIVETEXTUREPROC           glActiveTexture_ptr           = nullptr;

#if defined(_WIN32)
static void *gpur_get_proc_address(const char *name)
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
static void *gpur_get_proc_address(const char *name)
{
    return reinterpret_cast<void *>(glXGetProcAddressARB(
        reinterpret_cast<const GLubyte *>(name)));
}
#endif

template <typename Fn>
static bool gpur_load_symbol(Fn &symbol, const char *name)
{
    void *proc = gpur_get_proc_address(name);
    if (proc == nullptr)
        return false;
    std::memcpy(&symbol, &proc, sizeof(symbol));
    return true;
}

#define GPUR_LOAD(name) \
    if (!gpur_load_symbol(name, #name)) return false

bool gpur_load_gl_functions() noexcept
{
    GPUR_LOAD(glGenVertexArrays);
    GPUR_LOAD(glBindVertexArray);
    GPUR_LOAD(glDeleteVertexArrays);
    GPUR_LOAD(glGenBuffers);
    GPUR_LOAD(glBindBuffer);
    GPUR_LOAD(glBufferData);
    GPUR_LOAD(glDeleteBuffers);
    GPUR_LOAD(glVertexAttribIPointer);
    GPUR_LOAD(glEnableVertexAttribArray);
    GPUR_LOAD(glCreateShader);
    GPUR_LOAD(glShaderSource);
    GPUR_LOAD(glCompileShader);
    GPUR_LOAD(glGetShaderiv);
    GPUR_LOAD(glGetShaderInfoLog);
    GPUR_LOAD(glDeleteShader);
    GPUR_LOAD(glCreateProgram);
    GPUR_LOAD(glAttachShader);
    GPUR_LOAD(glLinkProgram);
    GPUR_LOAD(glGetProgramiv);
    GPUR_LOAD(glGetProgramInfoLog);
    GPUR_LOAD(glUseProgram);
    GPUR_LOAD(glDeleteProgram);
    GPUR_LOAD(glGetUniformLocation);
    GPUR_LOAD(glUniformMatrix4fv);
    GPUR_LOAD(glUniform1i);
    GPUR_LOAD(glUniform3fv);
    GPUR_LOAD(glUniform4fv);
    GPUR_LOAD(glUniform2f);
    GPUR_LOAD(glUniform1f);
    GPUR_LOAD(glGenerateMipmap);
    {
        void *proc = gpur_get_proc_address("glBufferSubData");
        if (proc == nullptr)
            return false;
        std::memcpy(&glBufferSubData_ptr, &proc, sizeof(proc));
    }
    {
        void *proc = gpur_get_proc_address("glVertexAttribPointer");
        if (proc == nullptr)
            return false;
        std::memcpy(&glVertexAttribPointer_ptr, &proc, sizeof(proc));
    }
    GPUR_LOAD(glActiveTexture);
    return true;
}

#undef GPUR_LOAD

#endif
