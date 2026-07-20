#include "../../Modules/Game/game_resistance.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdint>

FT_TEST(test_game_resistance_accepts_zero_percent)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_percent(0));
    FT_ASSERT_EQ(0, value.get_percent());
    return (1);
}

FT_TEST(test_game_resistance_accepts_negative_percent)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_percent(-1));
    FT_ASSERT_EQ(-1, value.get_percent());
    return (1);
}

FT_TEST(test_game_resistance_accepts_percent_above_one_hundred)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_percent(101));
    FT_ASSERT_EQ(101, value.get_percent());
    return (1);
}

FT_TEST(test_game_resistance_accepts_negative_flat_value)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_flat(-10));
    FT_ASSERT_EQ(-10, value.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_accepts_large_flat_value)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_flat(INT32_MAX));
    FT_ASSERT_EQ(INT32_MAX, value.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_set_values_preserves_negative_inputs)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(-5, -8));
    FT_ASSERT_EQ(-5, value.get_percent());
    FT_ASSERT_EQ(-8, value.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_reset_after_boundary_values)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(101, -20));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.reset());
    FT_ASSERT_EQ(0, value.get_percent());
    FT_ASSERT_EQ(0, value.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_boundary_values_survive_thread_safety)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(101, -20));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(101, value.get_percent());
    FT_ASSERT_EQ(-20, value.get_flat());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_resistance_set_values_updates_both_boundaries)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_values(0, INT32_MAX));
    FT_ASSERT_EQ(0, value.get_percent());
    FT_ASSERT_EQ(INT32_MAX, value.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_repeated_reset_is_safe)
{
    game_resistance value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.reset());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.reset());
    FT_ASSERT_EQ(0, value.get_percent());
    FT_ASSERT_EQ(0, value.get_flat());
    return (1);
}
