#include "../test_internal.hpp"
#include "../../Modules/GPGR/ft_gpu_window.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include <cstdlib>

static ft_gpu_window *g_gpgr_platform_window_pointer = nullptr;

static void gpgr_platform_set_cursor_visible_destroyed(void)
{
    g_gpgr_platform_window_pointer->set_cursor_visible(FT_FALSE);
    return ;
}

FT_TEST(test_gpgr_platform_window_smoke_when_enabled)
{
    const char *integration_enabled;
    ft_gpu_window *window_pointer;
    ft_bool initialized;

    integration_enabled = std::getenv("FT_GPGR_INTEGRATION");
    if (integration_enabled == nullptr
        || std::strcmp(integration_enabled, "1") != 0)
        return (1);
    window_pointer = ft_gpu_window::create();
    FT_ASSERT(window_pointer != nullptr);
    initialized = window_pointer->initialize("libft GPGR test", 320, 240,
        FT_FALSE);
    FT_ASSERT_EQ(FT_TRUE, initialized);
    FT_ASSERT_EQ(320, window_pointer->get_width());
    FT_ASSERT_EQ(240, window_pointer->get_height());
    FT_ASSERT_EQ(FT_FALSE, window_pointer->should_close());
    window_pointer->set_cursor_visible(FT_FALSE);
    window_pointer->set_cursor_visible(FT_TRUE);
    window_pointer->poll_events();
    window_pointer->swap_buffers();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_pointer->destroy());
    delete window_pointer;
    return (1);
}

FT_TEST(test_gpgr_platform_window_move_when_enabled)
{
    const char *integration_enabled;
    ft_gpu_window *source_window;
    ft_gpu_window *destination_window;

    integration_enabled = std::getenv("FT_GPGR_INTEGRATION");
    if (integration_enabled == nullptr
        || std::strcmp(integration_enabled, "1") != 0)
        return (1);
    source_window = ft_gpu_window::create();
    destination_window = ft_gpu_window::create();
    FT_ASSERT(source_window != nullptr);
    FT_ASSERT(destination_window != nullptr);
    FT_ASSERT_EQ(FT_TRUE, source_window->initialize("libft GPGR source",
        320, 240, FT_FALSE));
    FT_ASSERT_EQ(FT_TRUE, destination_window->initialize(
        "libft GPGR destination", 640, 480, FT_FALSE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_window->move(*source_window));
    FT_ASSERT_EQ(320, destination_window->get_width());
    FT_ASSERT_EQ(240, destination_window->get_height());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_window->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_window->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_window->destroy());
    delete source_window;
    delete destination_window;
    return (1);
}

FT_TEST(test_gpgr_platform_window_cursor_visibility_when_enabled)
{
    const char *integration_enabled;
    ft_gpu_window *window_pointer;

    integration_enabled = std::getenv("FT_GPGR_INTEGRATION");
    if (integration_enabled == nullptr
        || std::strcmp(integration_enabled, "1") != 0)
        return (1);
    window_pointer = ft_gpu_window::create();
    FT_ASSERT(window_pointer != nullptr);
    FT_ASSERT_EQ(FT_TRUE, window_pointer->initialize("libft GPGR cursor",
        320, 240, FT_FALSE));
    window_pointer->set_cursor_visible(FT_FALSE);
    window_pointer->set_cursor_visible(FT_FALSE);
    window_pointer->set_cursor_visible(FT_TRUE);
    window_pointer->set_cursor_visible(FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_pointer->destroy());
    delete window_pointer;
    return (1);
}

FT_TEST(test_gpgr_platform_window_destroyed_cursor_visibility_aborts_when_enabled)
{
    const char *integration_enabled;

    integration_enabled = std::getenv("FT_GPGR_INTEGRATION");
    if (integration_enabled == nullptr
        || std::strcmp(integration_enabled, "1") != 0)
        return (1);
    g_gpgr_platform_window_pointer = ft_gpu_window::create();
    FT_ASSERT(g_gpgr_platform_window_pointer != nullptr);
    FT_ASSERT_EQ(FT_TRUE, g_gpgr_platform_window_pointer->initialize(
        "libft GPGR destroyed cursor", 320, 240, FT_FALSE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        g_gpgr_platform_window_pointer->destroy());
    FT_ASSERT_EQ(1, test_expect_sigabrt_signal(
        gpgr_platform_set_cursor_visible_destroyed));
    delete g_gpgr_platform_window_pointer;
    g_gpgr_platform_window_pointer = nullptr;
    return (1);
}
