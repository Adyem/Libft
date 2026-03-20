#include "../test_internal.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/game_region_definition.hpp"
#include "../../Game/game_world_region.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void world_registry_fetch_region_after_destroy_aborts()
{
    game_world_registry registry;
    game_region_definition fetched_region;

    (void)registry.initialize();
    (void)registry.destroy();
    (void)registry.fetch_region(12, fetched_region);
    return ;
}

FT_TEST(test_world_registry_fetch_missing_entries_returns_not_found)
{
    game_world_registry registry;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    game_region_definition region;
    game_world_region world;

    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(100, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(200, world));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    return (1);
}

FT_TEST(test_world_registry_clears_entries_after_destroy)
{
    game_world_registry registry;
    ft_vector<int> region_ids;
    game_region_definition region;
    game_region_definition fetched_region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.push_back(12));
    game_world_region world;

    ft_string fjord_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fjord_name.initialize("Fjord"));
    ft_string icy_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, icy_description.initialize("Icy"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(12, fjord_name,
        icy_description, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(5, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.register_world(world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(1, test_expect_sigabrt_signal(
        world_registry_fetch_region_after_destroy_aborts));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.destroy());
    return (1);
}
