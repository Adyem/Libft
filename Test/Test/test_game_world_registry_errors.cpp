#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_fetch_missing_entries_returns_not_found,
        "fetching unknown ids returns FT_ERR_NOT_FOUND")
{
    ft_world_registry registry;
    ft_region_definition region;
    ft_world_region world;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(100, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(200, world));
    return (1);
}

FT_TEST(test_world_registry_clears_entries_after_destroy,
        "destroying registry empties stored data")
{
    ft_world_registry registry;
    ft_vector<int> region_ids;
    ft_region_definition region;
    ft_region_definition fetched_region;

    region_ids.push_back(12);
    ft_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(12, ft_string("Fjord"),
        ft_string("Icy"), 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(5, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.destroy());

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, registry.fetch_region(12, fetched_region));
    return (1);
}
