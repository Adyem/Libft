#include "../test_internal.hpp"
#include "../../Modules/GPGR/ft_gpu_shader.hpp"
#include "../../Modules/GPGR/gpgr_gl_funcs.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
# define GPGR_TEST_APIENTRY APIENTRY
#else
# define GPGR_TEST_APIENTRY
#endif

#if !defined(__APPLE__)

struct s_gpgr_fake_gl_state
{
    GLuint next_shader;
    GLuint next_program;
    GLuint last_created_program;
    GLuint last_used_program;
    GLuint last_uniform_program;
    GLint uniform_result;
    uint32_t create_shader_calls;
    uint32_t shader_source_calls;
    uint32_t compile_shader_calls;
    uint32_t get_shader_parameter_calls;
    uint32_t delete_shader_calls;
    uint32_t create_program_calls;
    uint32_t attach_shader_calls;
    uint32_t link_program_calls;
    uint32_t get_program_parameter_calls;
    uint32_t delete_program_calls;
    uint32_t use_program_calls;
    uint32_t uniform_calls;
    uint32_t deleted_shader_ids[8];
    uint32_t deleted_program_ids[8];
    char last_uniform_name[64];
    ft_bool fail_create_shader;
    ft_bool fail_vertex_compile;
    ft_bool fail_fragment_compile;
    ft_bool fail_create_program;
    ft_bool fail_link;
    GLuint shader_ids[8];
    GLenum shader_types[8];
    uint32_t shader_count;
};

static s_gpgr_fake_gl_state g_gpgr_fake_gl_state;

static int32_t gpgr_fake_shader_index(GLuint shader)
{
    uint32_t index = 0U;

    while (index < g_gpgr_fake_gl_state.shader_count)
    {
        if (g_gpgr_fake_gl_state.shader_ids[index] == shader)
            return (static_cast<int32_t>(index));
        index++;
    }
    return (-1);
}

static void gpgr_fake_gl_reset(void)
{
    std::memset(&g_gpgr_fake_gl_state, 0, sizeof(g_gpgr_fake_gl_state));
    g_gpgr_fake_gl_state.next_shader = 100U;
    g_gpgr_fake_gl_state.next_program = 500U;
    g_gpgr_fake_gl_state.uniform_result = 17;
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
    return ;
}

static GLuint GPGR_TEST_APIENTRY gpgr_fake_gl_create_shader(GLenum type)
{
    GLuint shader;

    g_gpgr_fake_gl_state.create_shader_calls++;
    if (g_gpgr_fake_gl_state.fail_create_shader == FT_TRUE)
        return (0U);
    shader = g_gpgr_fake_gl_state.next_shader++;
    if (g_gpgr_fake_gl_state.shader_count < 8U)
    {
        g_gpgr_fake_gl_state.shader_ids[g_gpgr_fake_gl_state.shader_count] = shader;
        g_gpgr_fake_gl_state.shader_types[g_gpgr_fake_gl_state.shader_count] = type;
        g_gpgr_fake_gl_state.shader_count++;
    }
    return (shader);
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_shader_source(GLuint shader,
    GLsizei count, const GLchar *const *source, const GLint *length)
{
    (void)shader;
    (void)source;
    (void)length;
    g_gpgr_fake_gl_state.shader_source_calls += static_cast<uint32_t>(count);
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_compile_shader(GLuint shader)
{
    (void)shader;
    g_gpgr_fake_gl_state.compile_shader_calls++;
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_get_shaderiv(GLuint shader,
    GLenum parameter_name, GLint *parameter_value)
{
    int32_t shader_index;
    GLenum shader_type;

    g_gpgr_fake_gl_state.get_shader_parameter_calls++;
    if (parameter_value == nullptr)
        return ;
    *parameter_value = 1;
    if (parameter_name != GL_COMPILE_STATUS)
        return ;
    shader_index = gpgr_fake_shader_index(shader);
    if (shader_index < 0)
        return ;
    shader_type = g_gpgr_fake_gl_state.shader_types[shader_index];
    if ((shader_type == GL_VERTEX_SHADER
            && g_gpgr_fake_gl_state.fail_vertex_compile == FT_TRUE)
        || (shader_type == GL_FRAGMENT_SHADER
            && g_gpgr_fake_gl_state.fail_fragment_compile == FT_TRUE))
    {
        *parameter_value = 0;
    }
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_get_shader_info_log(GLuint shader,
    GLsizei maximum_length, GLsizei *length, GLchar *info_log)
{
    const char *message = "fake shader compilation failure";

    (void)shader;
    if (length != nullptr)
        *length = 0;
    if (info_log == nullptr || maximum_length <= 0)
        return ;
    std::snprintf(info_log, static_cast<size_t>(maximum_length), "%s", message);
    if (length != nullptr)
        *length = static_cast<GLsizei>(std::strlen(info_log));
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_delete_shader(GLuint shader)
{
    uint32_t index = g_gpgr_fake_gl_state.delete_shader_calls;

    g_gpgr_fake_gl_state.delete_shader_calls++;
    if (index < 8U)
        g_gpgr_fake_gl_state.deleted_shader_ids[index] = shader;
    return ;
}

static GLuint GPGR_TEST_APIENTRY gpgr_fake_gl_create_program(void)
{
    GLuint program;

    g_gpgr_fake_gl_state.create_program_calls++;
    if (g_gpgr_fake_gl_state.fail_create_program == FT_TRUE)
        return (0U);
    program = g_gpgr_fake_gl_state.next_program++;
    g_gpgr_fake_gl_state.last_created_program = program;
    return (program);
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_attach_shader(GLuint program,
    GLuint shader)
{
    (void)program;
    (void)shader;
    g_gpgr_fake_gl_state.attach_shader_calls++;
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_link_program(GLuint program)
{
    (void)program;
    g_gpgr_fake_gl_state.link_program_calls++;
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_get_programiv(GLuint program,
    GLenum parameter_name, GLint *parameter_value)
{
    (void)program;
    g_gpgr_fake_gl_state.get_program_parameter_calls++;
    if (parameter_value == nullptr)
        return ;
    *parameter_value = 1;
    if (parameter_name == GL_LINK_STATUS
        && g_gpgr_fake_gl_state.fail_link == FT_TRUE)
    {
        *parameter_value = 0;
    }
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_get_program_info_log(
    GLuint program, GLsizei maximum_length, GLsizei *length, GLchar *info_log)
{
    const char *message = "fake program link failure";

    (void)program;
    if (length != nullptr)
        *length = 0;
    if (info_log == nullptr || maximum_length <= 0)
        return ;
    std::snprintf(info_log, static_cast<size_t>(maximum_length), "%s", message);
    if (length != nullptr)
        *length = static_cast<GLsizei>(std::strlen(info_log));
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_delete_program(GLuint program)
{
    uint32_t index = g_gpgr_fake_gl_state.delete_program_calls;

    g_gpgr_fake_gl_state.delete_program_calls++;
    if (index < 8U)
        g_gpgr_fake_gl_state.deleted_program_ids[index] = program;
    return ;
}

static void GPGR_TEST_APIENTRY gpgr_fake_gl_use_program(GLuint program)
{
    g_gpgr_fake_gl_state.use_program_calls++;
    g_gpgr_fake_gl_state.last_used_program = program;
    return ;
}

static GLint GPGR_TEST_APIENTRY gpgr_fake_gl_get_uniform_location(
    GLuint program, const GLchar *name)
{
    g_gpgr_fake_gl_state.uniform_calls++;
    g_gpgr_fake_gl_state.last_uniform_program = program;
    g_gpgr_fake_gl_state.last_uniform_name[0] = '\0';
    if (name == nullptr)
        return (-1);
    std::snprintf(g_gpgr_fake_gl_state.last_uniform_name,
        sizeof(g_gpgr_fake_gl_state.last_uniform_name), "%s", name);
    if (std::strcmp(name, "u_model") == 0)
        return (g_gpgr_fake_gl_state.uniform_result);
    return (-1);
}

static void gpgr_fake_gl_install(void)
{
    glCreateShader_ptr = gpgr_fake_gl_create_shader;
    glShaderSource_ptr = gpgr_fake_gl_shader_source;
    glCompileShader_ptr = gpgr_fake_gl_compile_shader;
    glGetShaderiv_ptr = gpgr_fake_gl_get_shaderiv;
    glGetShaderInfoLog_ptr = gpgr_fake_gl_get_shader_info_log;
    glDeleteShader_ptr = gpgr_fake_gl_delete_shader;
    glCreateProgram_ptr = gpgr_fake_gl_create_program;
    glAttachShader_ptr = gpgr_fake_gl_attach_shader;
    glLinkProgram_ptr = gpgr_fake_gl_link_program;
    glGetProgramiv_ptr = gpgr_fake_gl_get_programiv;
    glGetProgramInfoLog_ptr = gpgr_fake_gl_get_program_info_log;
    glUseProgram_ptr = gpgr_fake_gl_use_program;
    glDeleteProgram_ptr = gpgr_fake_gl_delete_program;
    glGetUniformLocation_ptr = gpgr_fake_gl_get_uniform_location;
    return ;
}

static int32_t gpgr_fake_shader_initialize(ft_gpu_shader &shader)
{
    const char *vertex_source = "void main() { gl_Position = vec4(0.0); }";
    const char *fragment_source = "void main() { }";

    return (shader.initialize(vertex_source, fragment_source));
}

FT_TEST(test_gpgr_shader_default_state)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_rejects_null_vertex_source)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, shader.initialize(nullptr,
        "void main() { }"));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.create_shader_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_rejects_null_fragment_source)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, shader.initialize(
        "void main() { }", nullptr));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.create_shader_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_initializes_and_reports_ready)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_TRUE, shader.ready());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.create_shader_calls);
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.shader_source_calls);
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.compile_shader_calls);
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.create_program_calls);
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.attach_shader_calls);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.link_program_calls);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_use_and_uniform_forward_to_gl)
{
    ft_gpu_shader shader;
    GLuint program;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    program = g_gpgr_fake_gl_state.last_created_program;
    shader.use();
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.use_program_calls);
    FT_ASSERT_EQ(program, g_gpgr_fake_gl_state.last_used_program);
    FT_ASSERT_EQ(17, shader.uniform("u_model"));
    FT_ASSERT_EQ(program, g_gpgr_fake_gl_state.last_uniform_program);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.uniform_calls);
    FT_ASSERT_EQ(0, std::strcmp("u_model",
        g_gpgr_fake_gl_state.last_uniform_name));
    FT_ASSERT_EQ(-1, shader.uniform("u_missing"));
    FT_ASSERT_EQ(-1, shader.uniform(nullptr));
    FT_ASSERT_EQ(3U, g_gpgr_fake_gl_state.uniform_calls);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    return (1);
}

FT_TEST(test_gpgr_shader_vertex_compile_failure_cleans_up)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    g_gpgr_fake_gl_state.fail_vertex_compile = FT_TRUE;
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INITIALIZATION_FAILED,
        gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.create_shader_calls);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.create_program_calls);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    return (1);
}

FT_TEST(test_gpgr_shader_fragment_compile_failure_cleans_up_both_stages)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    g_gpgr_fake_gl_state.fail_fragment_compile = FT_TRUE;
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INITIALIZATION_FAILED,
        gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.create_shader_calls);
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.create_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_creation_failure_leaves_no_resources)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    g_gpgr_fake_gl_state.fail_create_shader = FT_TRUE;
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INITIALIZATION_FAILED,
        gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.create_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.create_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_program_creation_failure_cleans_up_stages)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    g_gpgr_fake_gl_state.fail_create_program = FT_TRUE;
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.attach_shader_calls);
    FT_ASSERT_EQ(0U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_link_failure_cleans_up_program)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    g_gpgr_fake_gl_state.fail_link = FT_TRUE;
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_INITIALIZATION_FAILED,
        gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_shader_calls);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_destroy_is_idempotent)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    FT_ASSERT_EQ(FT_FALSE, shader.ready());
    return (1);
}

FT_TEST(test_gpgr_shader_reinitializes_after_destroy)
{
    ft_gpu_shader shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    FT_ASSERT_EQ(FT_TRUE, shader.ready());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.create_program_calls);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_move_transfers_program_and_state)
{
    ft_gpu_shader source_shader;
    ft_gpu_shader destination_shader;
    GLuint source_program;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(source_shader));
    source_program = g_gpgr_fake_gl_state.last_created_program;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.move(source_shader));
    FT_ASSERT_EQ(FT_TRUE, destination_shader.ready());
    FT_ASSERT_EQ(FT_FALSE, source_shader.ready());
    destination_shader.use();
    FT_ASSERT_EQ(source_program, g_gpgr_fake_gl_state.last_used_program);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.destroy());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_move_replaces_destination)
{
    ft_gpu_shader source_shader;
    ft_gpu_shader destination_shader;
    GLuint source_program;
    GLuint destination_program;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(source_shader));
    source_program = g_gpgr_fake_gl_state.last_created_program;
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        gpgr_fake_shader_initialize(destination_shader));
    destination_program = g_gpgr_fake_gl_state.last_created_program;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.move(source_shader));
    FT_ASSERT_EQ(FT_TRUE, destination_shader.ready());
    FT_ASSERT_EQ(FT_FALSE, source_shader.ready());
    destination_shader.use();
    FT_ASSERT_EQ(source_program, g_gpgr_fake_gl_state.last_used_program);
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    FT_ASSERT_EQ(destination_program,
        g_gpgr_fake_gl_state.deleted_program_ids[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.destroy());
    FT_ASSERT_EQ(2U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_move_destroyed_source_keeps_destination_destroyed)
{
    ft_gpu_shader source_shader;
    ft_gpu_shader destination_shader;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(source_shader));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_shader.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.move(source_shader));
    FT_ASSERT_EQ(FT_FALSE, destination_shader.ready());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_shader.destroy());
    FT_ASSERT_EQ(1U, g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

FT_TEST(test_gpgr_shader_self_move_is_noop)
{
    ft_gpu_shader shader;
    uint32_t delete_program_calls;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
    delete_program_calls = g_gpgr_fake_gl_state.delete_program_calls;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.move(shader));
    FT_ASSERT_EQ(FT_TRUE, shader.ready());
    FT_ASSERT_EQ(delete_program_calls,
        g_gpgr_fake_gl_state.delete_program_calls);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shader.destroy());
    return (1);
}

FT_TEST(test_gpgr_shader_destructor_releases_program)
{
    uint32_t delete_program_calls;

    gpgr_fake_gl_reset();
    gpgr_fake_gl_install();
    {
        ft_gpu_shader shader;

        FT_ASSERT_EQ(FT_ERR_SUCCESS, gpgr_fake_shader_initialize(shader));
        delete_program_calls = g_gpgr_fake_gl_state.delete_program_calls;
    }
    FT_ASSERT_EQ(delete_program_calls + 1U,
        g_gpgr_fake_gl_state.delete_program_calls);
    return (1);
}

#endif
