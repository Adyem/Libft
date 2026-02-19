#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_tracks_regions_and_worlds, "registry stores multiple entries")
{
    ft_world_registry registry;
    ft_region_definition region1(1, ft_string("Camp"), ft_string("Camp grounds"), 1);
    ft_region_definition region2(2, ft_string("Ruins"), ft_string("Old ruins"), 4);
    ft_vector<int> region_ids1;
    ft_vector<int> region_ids2;
    region_ids1.push_back(1);
    region_ids2.push_back(2);
    ft_world_region world1(10, region_ids1);
    ft_world_region world2(11, region_ids2);
    ft_region_definition fetched_region;
    ft_world_region fetched_world;


    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world2));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(1, fetched_region));
    FT_ASSERT_EQ(ft_string("Camp"), fetched_region.get_name());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(10, fetched_world));
    FT_ASSERT_EQ(1, fetched_world.get_world_id());
    return (1);
}
