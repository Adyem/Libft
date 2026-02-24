#include "../test_internal.hpp"
#include "../../Game/game_resistance.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_resistance_default_initialization, "Game: resistance defaults to zeroed values")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_set_percent_resets_errno, "Game: set_percent sets errno to success")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.set_percent(15));
    FT_ASSERT_EQ(15, resistance.get_percent());
    return (1);
}

FT_TEST(test_game_resistance_set_flat_resets_errno, "Game: set_flat sets errno to success")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.set_flat(6));
    FT_ASSERT_EQ(6, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_set_values_updates_both, "Game: set_values replaces both percent and flat values")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.set_values(20, 8));
    FT_ASSERT_EQ(20, resistance.get_percent());
    FT_ASSERT_EQ(8, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_reset_clears_after_updates, "Game: reset clears previously assigned values")
{
    ft_resistance resistance;

    resistance.set_values(12, 4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.reset());
    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_get_percent_sets_errno_success, "Game: get_percent resets errno to success after read")
{
    ft_resistance resistance;

    resistance.set_percent(9);
    FT_ASSERT_EQ(9, resistance.get_percent());
    return (1);
}

FT_TEST(test_game_resistance_get_flat_sets_errno_success, "Game: get_flat resets errno to success after read")
{
    ft_resistance resistance;

    resistance.set_flat(13);
    FT_ASSERT_EQ(13, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_get_error_sets_errno_success, "Game: get_error clears errno to success")
{
    ft_resistance resistance;

    resistance.set_values(5, 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.get_error());
    return (1);
}

FT_TEST(test_game_resistance_get_error_str_reports_success, "Game: get_error_str returns success string and resets errno")
{
    ft_resistance resistance;

    resistance.set_values(3, 1);
    FT_ASSERT_STR_EQ(ft_strerror(FT_ERR_SUCCESS), resistance.get_error_str());
    return (1);
}

FT_TEST(test_game_resistance_thread_safety_toggle, "Game: resistance thread safety toggles explicitly")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(false, resistance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.enable_thread_safety());
    FT_ASSERT_EQ(true, resistance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.disable_thread_safety());
    FT_ASSERT_EQ(false, resistance.is_thread_safe());
    return (1);
}
