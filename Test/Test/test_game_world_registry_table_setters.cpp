#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_setters, "world registry setter copies replace maps")
{
    ft_world_registry registry;
    ft_map<int, ft_region_definition> regions;
    ft_map<int, ft_world_region> worlds;
    ft_vector<int> region_ids;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, regions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, worlds.initialize());
    regions.insert(2, ft_region_definition(2, ft_string("cave"), ft_string("dark"), 8));
    region_ids.push_back(2);
    worlds.insert(5, ft_world_region(5, region_ids));

    registry.set_regions(regions);
    registry.set_world_regions(worlds);
    regions.clear();
    worlds.clear();

    FT_ASSERT_EQ(1, registry.get_regions().size());
    FT_ASSERT_EQ(1, registry.get_world_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(2, fetched_region));
    FT_ASSERT_EQ(ft_string("cave"), fetched_region.get_name());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(5, fetched_world));
    FT_ASSERT_EQ(2, fetched_world.get_region_ids()[0]);

    registry.get_regions().clear();
    registry.get_world_regions().clear();
    FT_ASSERT_EQ(0, registry.get_regions().size());
    FT_ASSERT_EQ(0, registry.get_world_regions().size());
    registry.set_regions(regions);
    registry.set_world_regions(worlds);
    FT_ASSERT_EQ(0, registry.get_regions().size());
    FT_ASSERT_EQ(0, registry.get_world_regions().size());
    return (1);
}
