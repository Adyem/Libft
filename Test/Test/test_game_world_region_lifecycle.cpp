#include "../test_internal.hpp"

#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_world_region_thread_safe_lifecycle)
{
    game_world_region world_region;
    ft_vector<int32_t> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        world_region.initialize(44, region_ids));
    FT_ASSERT_EQ(44, world_region.get_world_id());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), world_region.get_region_ids().size());
    region_ids.push_back(2);
    region_ids.push_back(3);
    world_region.set_region_ids(region_ids);
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), world_region.get_region_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_region.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.destroy());
    return (1);
}
