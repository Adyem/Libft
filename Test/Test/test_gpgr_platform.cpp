#include "../test_internal.hpp"
#include "../../Modules/GPGR/ft_gpu_window.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include <cstdlib>

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

