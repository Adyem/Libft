#include "../test_internal.hpp"

#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_world_lifecycle)
{
    game_world world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT(world.get_event_scheduler().get() != ft_nullptr);
    FT_ASSERT(world.get_world_registry().get() != ft_nullptr);
    FT_ASSERT(world.get_replay_session().get() != ft_nullptr);
    FT_ASSERT(world.get_dialogue_table().get() != ft_nullptr);
    FT_ASSERT(world.get_world_region().get() != ft_nullptr);
    FT_ASSERT(world.get_quest().get() != ft_nullptr);
    FT_ASSERT(world.get_upgrade().get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.destroy());
    return (1);
}
