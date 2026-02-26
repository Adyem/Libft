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
    ft_region_definition region1;
    ft_region_definition region2;
    ft_vector<int> region_ids1;
    ft_vector<int> region_ids2;
    region_ids1.push_back(1);
    region_ids2.push_back(2);
    ft_world_region world1;
    ft_world_region world2;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.get_error());
    region1.set_region_id(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.get_error());
    ft_string camp_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, camp_name.initialize("Camp"));
    region1.set_name(camp_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.get_error());
    ft_string camp_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, camp_description.initialize("Camp grounds"));
    region1.set_description(camp_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.get_error());
    region1.set_recommended_level(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region1.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.get_error());
    region2.set_region_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.get_error());
    ft_string ruins_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ruins_name.initialize("Ruins"));
    region2.set_name(ruins_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.get_error());
    ft_string ruins_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ruins_description.initialize("Old ruins"));
    region2.set_description(ruins_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.get_error());
    region2.set_recommended_level(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region2.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world1.initialize(10, region_ids1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world1.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world2.initialize(11, region_ids2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world2.get_error());


    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(1, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("Camp", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(10, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(10, fetched_world.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    return (1);
}
