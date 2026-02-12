#include "../test_internal.hpp"
#include "../../DUMB/dumb_render.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_render_window render_window_type;

static int render_expect_sigabrt_uninitialized(void (*operation)(render_window_type &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(render_window_type) unsigned char storage[sizeof(render_window_type)];
        render_window_type *render_window_pointer;

        std::memset(storage, 0, sizeof(storage));
        render_window_pointer = reinterpret_cast<render_window_type *>(storage);
        operation(*render_window_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void render_call_destructor(render_window_type &render_window_instance)
{
    render_window_instance.~render_window_type();
    return ;
}

static void render_call_destroy(render_window_type &render_window_instance)
{
    (void)render_window_instance.destroy();
    return ;
}

static void render_call_initialize_desc(render_window_type &render_window_instance)
{
    ft_render_window_desc description;

    description.width = 1;
    description.height = 1;
    description.title = "x";
    description.flags = ft_render_window_flag_none;
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
    (void)render_window_instance.set_fullscreen(false);
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
    (void)render_window_instance.is_thread_safe_enabled();
    return ;
}

static void render_call_should_close(render_window_type &render_window_instance)
{
    (void)render_window_instance.should_close();
    return ;
}

static void render_call_runtime_mutex(render_window_type &render_window_instance)
{
    (void)render_window_instance.runtime_mutex();
    return ;
}

FT_TEST(test_dumb_render_uninitialized_destructor_aborts,
    "dumb render destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_destructor));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_destroy_aborts,
    "dumb render destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_destroy));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_initialize_desc_aborts,
    "dumb render initialize(desc) aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_initialize_desc));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_shutdown_aborts,
    "dumb render shutdown aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_shutdown));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_poll_events_aborts,
    "dumb render poll_events aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_poll_events));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_present_aborts,
    "dumb render present aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_present));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_framebuffer_aborts,
    "dumb render framebuffer aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_framebuffer));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_clear_aborts,
    "dumb render clear aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_clear));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_put_pixel_aborts,
    "dumb render put_pixel aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_put_pixel));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_set_fullscreen_aborts,
    "dumb render set_fullscreen aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_set_fullscreen));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_enable_thread_safety_aborts,
    "dumb render enable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_disable_thread_safety_aborts,
    "dumb render disable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_is_thread_safe_aborts,
    "dumb render is_thread_safe_enabled aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_is_thread_safe));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_should_close_aborts,
    "dumb render should_close aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_should_close));
    return (1);
}

FT_TEST(test_dumb_render_uninitialized_runtime_mutex_aborts,
    "dumb render runtime_mutex aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, render_expect_sigabrt_uninitialized(render_call_runtime_mutex));
    return (1);
}
