#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_quest_initialize_twice(game_quest &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_rarity_band_initialize_twice(game_rarity_band &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_quest_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_quest>(
                        game_quest_initialize_twice));
    return (1);
}

FT_TEST(test_game_rarity_band_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_rarity_band>(
                        game_rarity_band_initialize_twice));
    return (1);
}
