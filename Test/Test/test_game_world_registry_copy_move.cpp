#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_register_fetch_round_trip, "world registry stores regions and worlds")
{
    ft_world_registry registry;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    ft_vector<int> region_ids;
    ft_region_definition region;
    ft_region_definition fetched_region;
    ft_world_region world;
    ft_world_region fetched_world;

    region_ids.push_back(10);
    ft_string region_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_name.initialize("Harbor"));
    ft_string region_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_description.initialize("Seaside"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(10, region_name,
        region_description, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(10, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("Harbor", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(3, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(3, fetched_world.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    FT_ASSERT_EQ(1u, fetched_world.get_region_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    FT_ASSERT_EQ(10, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    return (1);
}
