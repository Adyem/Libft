#include "../test_internal.hpp"
#include "../../Modules/GPGR/gpgr_gl_funcs.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#if !defined(__APPLE__)

static void gpgr_test_set_stale_function_pointers(void)
{
    glGenVertexArrays_ptr = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
        static_cast<uintptr_t>(1U));
    glBindVertexArray_ptr = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
        static_cast<uintptr_t>(1U));
    glDeleteVertexArrays_ptr = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(
        static_cast<uintptr_t>(1U));
    glGenBuffers_ptr = reinterpret_cast<PFNGLGENBUFFERSPROC>(
        static_cast<uintptr_t>(1U));
    glBindBuffer_ptr = reinterpret_cast<PFNGLBINDBUFFERPROC>(
        static_cast<uintptr_t>(1U));
    glBufferData_ptr = reinterpret_cast<PFNGLBUFFERDATAPROC>(
        static_cast<uintptr_t>(1U));
    glBufferSubData_ptr = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(
        static_cast<uintptr_t>(1U));
    glDeleteBuffers_ptr = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(
        static_cast<uintptr_t>(1U));
    glVertexAttribIPointer_ptr = reinterpret_cast<PFNGLVERTEXATTRIBIPOINTERPROC>(
        static_cast<uintptr_t>(1U));
    glVertexAttribPointer_ptr = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
        static_cast<uintptr_t>(1U));
    glEnableVertexAttribArray_ptr = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
        static_cast<uintptr_t>(1U));
    glCreateShader_ptr = reinterpret_cast<PFNGLCREATESHADERPROC>(
        static_cast<uintptr_t>(1U));
    glShaderSource_ptr = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
        static_cast<uintptr_t>(1U));
    glCompileShader_ptr = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
        static_cast<uintptr_t>(1U));
    glGetShaderiv_ptr = reinterpret_cast<PFNGLGETSHADERIVPROC>(
        static_cast<uintptr_t>(1U));
    glGetShaderInfoLog_ptr = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
        static_cast<uintptr_t>(1U));
    glDeleteShader_ptr = reinterpret_cast<PFNGLDELETESHADERPROC>(
        static_cast<uintptr_t>(1U));
    glCreateProgram_ptr = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
        static_cast<uintptr_t>(1U));
    glAttachShader_ptr = reinterpret_cast<PFNGLATTACHSHADERPROC>(
        static_cast<uintptr_t>(1U));
    glLinkProgram_ptr = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
        static_cast<uintptr_t>(1U));
    glGetProgramiv_ptr = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
        static_cast<uintptr_t>(1U));
    glGetProgramInfoLog_ptr = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
        static_cast<uintptr_t>(1U));
    glUseProgram_ptr = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        static_cast<uintptr_t>(1U));
    glDeleteProgram_ptr = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
        static_cast<uintptr_t>(1U));
    glGetUniformLocation_ptr = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(
        static_cast<uintptr_t>(1U));
    glActiveTexture_ptr = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(
        static_cast<uintptr_t>(1U));
    return ;
}

FT_TEST(test_gpgr_loader_rejects_missing_current_context)
{
    ft_bool loaded;

    glGenVertexArrays_ptr = nullptr;
    glCreateShader_ptr = nullptr;
    glActiveTexture_ptr = nullptr;
    loaded = gpgr_load_gl_functions();
    FT_ASSERT_EQ(FT_FALSE, loaded);
    return (1);
}

FT_TEST(test_gpgr_loader_clears_partial_load_after_failure)
{
    gpgr_test_set_stale_function_pointers();
    FT_ASSERT_EQ(FT_FALSE, gpgr_load_gl_functions());
    FT_ASSERT(glGenVertexArrays_ptr == nullptr);
    FT_ASSERT(glBindVertexArray_ptr == nullptr);
    FT_ASSERT(glDeleteVertexArrays_ptr == nullptr);
    FT_ASSERT(glGenBuffers_ptr == nullptr);
    FT_ASSERT(glBindBuffer_ptr == nullptr);
    FT_ASSERT(glBufferData_ptr == nullptr);
    FT_ASSERT(glBufferSubData_ptr == nullptr);
    FT_ASSERT(glDeleteBuffers_ptr == nullptr);
    FT_ASSERT(glVertexAttribIPointer_ptr == nullptr);
    FT_ASSERT(glVertexAttribPointer_ptr == nullptr);
    FT_ASSERT(glEnableVertexAttribArray_ptr == nullptr);
    FT_ASSERT(glCreateShader_ptr == nullptr);
    FT_ASSERT(glShaderSource_ptr == nullptr);
    FT_ASSERT(glCompileShader_ptr == nullptr);
    FT_ASSERT(glGetShaderiv_ptr == nullptr);
    FT_ASSERT(glGetShaderInfoLog_ptr == nullptr);
    FT_ASSERT(glDeleteShader_ptr == nullptr);
    FT_ASSERT(glCreateProgram_ptr == nullptr);
    FT_ASSERT(glAttachShader_ptr == nullptr);
    FT_ASSERT(glLinkProgram_ptr == nullptr);
    FT_ASSERT(glGetProgramiv_ptr == nullptr);
    FT_ASSERT(glGetProgramInfoLog_ptr == nullptr);
    FT_ASSERT(glUseProgram_ptr == nullptr);
    FT_ASSERT(glDeleteProgram_ptr == nullptr);
    FT_ASSERT(glGetUniformLocation_ptr == nullptr);
    FT_ASSERT(glActiveTexture_ptr == nullptr);
    return (1);
}

FT_TEST(test_gpgr_loader_is_retryable_after_failure)
{
    FT_ASSERT_EQ(FT_FALSE, gpgr_load_gl_functions());
    FT_ASSERT(glCreateProgram_ptr == nullptr);
    FT_ASSERT(glDeleteProgram_ptr == nullptr);
    FT_ASSERT(glGetUniformLocation_ptr == nullptr);
    return (1);
}

#endif
