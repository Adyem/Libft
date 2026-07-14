#include "../test_internal.hpp"
#include "../../Modules/GPGR/gpgr_gl_funcs.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#if !defined(__APPLE__)

static void gpgr_test_set_stale_function_pointers(void)
{
    glGenVertexArrays_ptr = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
        static_cast<uintptr_t>(1U));
    glCreateShader_ptr = reinterpret_cast<PFNGLCREATESHADERPROC>(
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
    FT_ASSERT(glCreateShader_ptr == nullptr);
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
