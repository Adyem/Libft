#include "../test_internal.hpp"
#include "../../Modules/GPGR/ft_gpu_window.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

static ft_gpu_window *g_gpgr_test_window_pointer = nullptr;

static void gpgr_window_get_width_uninitialised(void)
{
    (void)g_gpgr_test_window_pointer->get_width();
    return ;
}

static void gpgr_window_get_height_uninitialised(void)
{
    (void)g_gpgr_test_window_pointer->get_height();
    return ;
}

FT_TEST(test_gpgr_window_factory_create_and_destroy)
{
    ft_gpu_window *window_pointer;

    window_pointer = ft_gpu_window::create();
    FT_ASSERT(window_pointer != nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_pointer->destroy());
    delete window_pointer;
    return (1);
}

FT_TEST(test_gpgr_window_destroy_is_safe_before_initialization)
{
    ft_gpu_window *window_pointer;

    window_pointer = ft_gpu_window::create();
    FT_ASSERT(window_pointer != nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, window_pointer->destroy());
    delete window_pointer;
    return (1);
}

FT_TEST(test_gpgr_window_uninitialised_getters_abort)
{
    g_gpgr_test_window_pointer = ft_gpu_window::create();
    FT_ASSERT(g_gpgr_test_window_pointer != nullptr);
    FT_ASSERT_EQ(1, test_expect_sigabrt_signal(
        gpgr_window_get_width_uninitialised));
    FT_ASSERT_EQ(1, test_expect_sigabrt_signal(
        gpgr_window_get_height_uninitialised));
    delete g_gpgr_test_window_pointer;
    g_gpgr_test_window_pointer = nullptr;
    return (1);
}

FT_TEST(test_gpgr_window_factory_creates_independent_windows)
{
    ft_gpu_window *first_window;
    ft_gpu_window *second_window;

    first_window = ft_gpu_window::create();
    second_window = ft_gpu_window::create();
    FT_ASSERT(first_window != nullptr);
    FT_ASSERT(second_window != nullptr);
    FT_ASSERT(first_window != second_window);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_window->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_window->destroy());
    delete first_window;
    delete second_window;
    return (1);
}

FT_TEST(test_gpgr_window_move_destroyed_source)
{
    ft_gpu_window *source_window;
    ft_gpu_window *destination_window;

    source_window = ft_gpu_window::create();
    destination_window = ft_gpu_window::create();
    FT_ASSERT(source_window != nullptr);
    FT_ASSERT(destination_window != nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_window->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_window->move(*source_window));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_window->destroy());
    delete source_window;
    delete destination_window;
    return (1);
}
