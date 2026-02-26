#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_register_and_fetch_records, "registered records round-trip through fetch")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    region_ids.push_back(4);

    ft_region_definition region;
    ft_world_region world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string forest_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, forest_name.initialize("forest"));
    region.set_name(forest_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string forest_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, forest_description.initialize("dense trees"));
    region.set_description(forest_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(2, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(4, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("forest", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_STR_EQ("dense trees", fetched_region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(2, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(2, fetched_world.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    FT_ASSERT_EQ((size_t)1, fetched_world.get_region_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    FT_ASSERT_EQ(4, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    return (1);
}

FT_TEST(test_world_registry_records_isolate_fetched_values, "re-fetching records returns original state")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    region_ids.push_back(11);

    ft_region_definition region;
    ft_world_region world;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;
    ft_region_definition re_fetched_region;
    ft_world_region re_fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string volcano_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, volcano_name.initialize("volcano"));
    region.set_name(volcano_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string volcano_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, volcano_description.initialize("lava dome"));
    region.set_description(volcano_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    ft_string changed_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, changed_name.initialize("changed"));
    fetched_region.set_name(changed_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, re_fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("volcano", re_fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, re_fetched_region.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(3, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    fetched_world.get_region_ids()[0] = 77;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(3, re_fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(11, re_fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, re_fetched_world.get_error());
    return (1);
}

FT_TEST(test_world_registry_records_handles_missing_entries, "fetch commands return not found")
{
    ft_world_registry registry;
    ft_region_definition region;
    ft_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(99, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(101, world));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    return (1);
}
