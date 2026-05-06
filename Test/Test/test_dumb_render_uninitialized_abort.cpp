#include "../test_internal.hpp"
#include "../../Modules/DUMB/render_window.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_render_window render_window_type;

static sigjmp_buf g_render_jump_buffer;
static volatile sig_atomic_t g_render_signal;

static void render_sigabrt_handler(int32_t signal_number)
{
    g_render_signal = signal_number;
    siglongjmp(g_render_jump_buffer, 1);
}

static int32_t render_expect_sigabrt_uninitialised(void (*operation)(render_window_type &))
{
    struct sigaction new_action;
    struct sigaction old_action;
    int32_t error_pipe[2];
    int32_t stderr_backup;
    char error_output[512];
    ssize_t read_size;
    ft_bool has_lifecycle_error;
    int32_t result;

    std::memset(&new_action, 0, sizeof(new_action));
    std::memset(&old_action, 0, sizeof(old_action));
    new_action.sa_handler = render_sigabrt_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
        return (0);
    error_pipe[0] = -1;
    error_pipe[1] = -1;
    stderr_backup = -1;
    std::memset(error_output, 0, sizeof(error_output));
    if (pipe(error_pipe) != 0)
    {
        (void)sigaction(SIGABRT, &old_action, ft_nullptr);
        return (0);
    }
    stderr_backup = dup(STDERR_FILENO);
    if (stderr_backup < 0)
    {
        close(error_pipe[0]);
        close(error_pipe[1]);
        (void)sigaction(SIGABRT, &old_action, ft_nullptr);
        return (0);
    }
    (void)dup2(error_pipe[1], STDERR_FILENO);

    g_render_signal = 0;
    if (sigsetjmp(g_render_jump_buffer, 1) == 0)
    {
        alignas(render_window_type) unsigned char storage[sizeof(render_window_type)];
        render_window_type *render_window_pointer;

        std::memset(storage, 0, sizeof(storage));
        render_window_pointer = reinterpret_cast<render_window_type *>(storage);
        operation(*render_window_pointer);
        result = 0;
    }
    else
    {
        result = (g_render_signal == SIGABRT);
    }
    (void)dup2(stderr_backup, STDERR_FILENO);
    close(stderr_backup);
    close(error_pipe[1]);
    read_size = read(error_pipe[0], error_output, sizeof(error_output) - 1);
    close(error_pipe[0]);
    has_lifecycle_error = FT_FALSE;
    if (read_size > 0 && std::strstr(error_output, "lifecycle error:") != ft_nullptr)
        has_lifecycle_error = FT_TRUE;
    if (result != 0 && has_lifecycle_error == FT_FALSE)
        result = 0;

    (void)sigaction(SIGABRT, &old_action, ft_nullptr);
    return (result);
}

static void render_call_destructor(render_window_type &render_window_instance)
{
    render_window_instance.~render_window_type();
    return ;
}

static void render_call_initialize_desc(render_window_type &render_window_instance)
{
    ft_render_window_desc description;

    description.width = 1;
    description.height = 1;
    description.title = "x";
    description.flags = FT_RENDER_WINDOW_FLAG_NONE;
    (void)render_window_instance.initialize(description);
    return ;
}

static void render_call_shutdown(render_window_type &render_window_instance)
{
    render_window_instance.shutdown();
    return ;
}

static void render_call_poll_events(render_window_type &render_window_instance)
{
    (void)render_window_instance.poll_events();
    return ;
}

static void render_call_present(render_window_type &render_window_instance)
{
    (void)render_window_instance.present();
    return ;
}

static void render_call_framebuffer(render_window_type &render_window_instance)
{
    (void)render_window_instance.framebuffer();
    return ;
}

static void render_call_clear(render_window_type &render_window_instance)
{
    (void)render_window_instance.clear(0U);
    return ;
}

static void render_call_put_pixel(render_window_type &render_window_instance)
{
    (void)render_window_instance.put_pixel(0, 0, 0U);
    return ;
}

static void render_call_set_fullscreen(render_window_type &render_window_instance)
{
    (void)render_window_instance.set_fullscreen(FT_FALSE);
    return ;
}

static void render_call_enable_thread_safety(render_window_type &render_window_instance)
{
    (void)render_window_instance.enable_thread_safety();
    return ;
}

static void render_call_disable_thread_safety(render_window_type &render_window_instance)
{
    render_window_instance.disable_thread_safety();
    return ;
}

static void render_call_is_thread_safe(render_window_type &render_window_instance)
{
    (void)render_window_instance.is_thread_safe();
    return ;
}

static void render_call_should_close(render_window_type &render_window_instance)
{
    (void)render_window_instance.should_close();
    return ;
}

FT_TEST(test_dumb_render_uninitialised_destructor_tolerates_object)
{
    FT_ASSERT_EQ(0, render_expect_sigabrt_uninitialised(render_call_destructor));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_destroy_returns_invalid_state)
{
    render_window_type window_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_initialize_desc_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_initialize_desc));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_shutdown_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_shutdown));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_poll_events_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_poll_events));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_present_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_present));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_framebuffer_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_framebuffer));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_clear_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_clear));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_put_pixel_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_put_pixel));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_set_fullscreen_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_set_fullscreen));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_disable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_is_thread_safe));
    return (1);
}

FT_TEST(test_dumb_render_uninitialised_should_close_aborts)
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialised(render_call_should_close));
    return (1);
}
