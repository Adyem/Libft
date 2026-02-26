#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdint>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_register_and_fetch, "register and fetch regions and worlds")
{
    ft_world_registry registry;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    ft_vector<int> region_ids;
    ft_region_definition region;
    ft_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string region_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_name.initialize("meadow"));
    region.set_name(region_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string region_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_description.initialize("green fields"));
    region.set_description(region_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region_ids.push_back(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(1, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    ft_region_definition fetched_region;
    ft_world_region fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(7, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("meadow", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(1, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(1, fetched_world.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    FT_ASSERT_EQ(7, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    return (1);
}

FT_TEST(test_world_isolation, "fetched values remain isolated")
{
    ft_world_registry registry;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    ft_vector<int> region_ids;
    ft_region_definition region;
    ft_world_region world;

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
    region_ids.push_back(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(2, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    ft_region_definition fetched_again;
    ft_world_region world_again;
    ft_world_region fetched_world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, fetched_again));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    ft_string changed_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, changed_name.initialize("changed"));
    fetched_again.set_name(changed_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_again.get_error());
    ft_region_definition fresh_region;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, fresh_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("volcano", fresh_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fresh_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(2, world_again));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    world_again.get_region_ids()[0] = 99;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(2, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(11, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    return (1);
}

FT_TEST(test_world_missing_entries, "missing ids surface errors")
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
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(90, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(45, world));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    return (1);
}
