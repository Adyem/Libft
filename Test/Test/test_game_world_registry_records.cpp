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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    region.set_region_id(4);
    region.set_name(ft_string("forest"));
    region.set_description(ft_string("dense trees"));
    region.set_recommended_level(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(2, region_ids));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(4, fetched_region));
    FT_ASSERT_EQ(ft_string("forest"), fetched_region.get_name());
    FT_ASSERT_EQ(ft_string("dense trees"), fetched_region.get_description());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(2, fetched_world));
    FT_ASSERT_EQ(2, fetched_world.get_world_id());
    FT_ASSERT_EQ((size_t)1, fetched_world.get_region_ids().size());
    FT_ASSERT_EQ(4, fetched_world.get_region_ids()[0]);
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    region.set_region_id(11);
    region.set_name(ft_string("volcano"));
    region.set_description(ft_string("lava dome"));
    region.set_recommended_level(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, fetched_region));
    fetched_region.set_name(ft_string("changed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(11, re_fetched_region));
    FT_ASSERT_EQ(ft_string("volcano"), re_fetched_region.get_name());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(3, fetched_world));
    fetched_world.get_region_ids()[0] = 77;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(3, re_fetched_world));
    FT_ASSERT_EQ(11, re_fetched_world.get_region_ids()[0]);
    return (1);
}

FT_TEST(test_world_registry_records_handles_missing_entries, "fetch commands return not found")
{
    ft_world_registry registry;
    ft_region_definition region;
    ft_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(99, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(101, world));
    return (1);
}
