#include "../test_internal.hpp"
#include "../../Math/ft_cubic_spline.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_cubic_spline_initialize_destroy_cycle)
{
    ft_cubic_spline spline;

    FT_ASSERT_EQ(FT_CLASS_STATE_UNINITIALISED, spline._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.initialize());
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, spline._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.destroy());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, spline._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.destroy());
    return (1);
}

FT_TEST(test_math_cubic_spline_copy_move_initialise)
{
    ft_cubic_spline source;
    ft_cubic_spline copied;
    ft_cubic_spline moved;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.initialize(source));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, copied._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(static_cast<ft_cubic_spline &&>(source)));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, moved._initialised_state);
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, source._initialised_state);
    return (1);
}

FT_TEST(test_math_cubic_spline_thread_safety_toggle)
{
    ft_cubic_spline spline;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.initialize());
    FT_ASSERT_EQ(FT_FALSE, spline.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, spline.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, spline.is_thread_safe());
    return (1);
}
