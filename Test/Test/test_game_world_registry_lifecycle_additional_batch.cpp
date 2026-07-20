#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void registry_initialize_twice(game_world_registry &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void registry_move_uninitialised(game_world_registry &value)
{
    game_world_registry source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_world_registry_initialize_twice_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_registry>(
                        registry_initialize_twice));
    return (1);
}

FT_TEST(test_game_world_registry_move_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_registry>(
                        registry_move_uninitialised));
    return (1);
}

FT_TEST(test_game_world_registry_default_maps_are_empty)
{
    game_world_registry value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_regions().size());
    FT_ASSERT_EQ(0, value.get_world_regions().size());
    return (1);
}

FT_TEST(test_game_world_registry_const_maps_are_empty)
{
    game_world_registry value;
    const game_world_registry &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_regions().size());
    FT_ASSERT_EQ(0, const_value.get_world_regions().size());
    return (1);
}

FT_TEST(test_game_world_registry_destroy_twice_is_safe_again)
{
    game_world_registry value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_world_registry_reinitialize_after_destroy_again)
{
    game_world_registry value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_regions().size());
    return (1);
}

FT_TEST(test_game_world_registry_set_empty_regions)
{
    game_world_registry value;
    ft_map<int32_t, game_region_definition> regions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regions.initialize());
    value.set_regions(regions);
    FT_ASSERT_EQ(0, value.get_regions().size());
    return (1);
}

FT_TEST(test_game_world_registry_set_empty_world_regions)
{
    game_world_registry value;
    ft_map<int32_t, game_world_region> world_regions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_regions.initialize());
    value.set_world_regions(world_regions);
    FT_ASSERT_EQ(0, value.get_world_regions().size());
    return (1);
}

FT_TEST(test_game_world_registry_empty_region_fetch_is_not_found)
{
    game_world_registry value;
    game_region_definition region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.fetch_region(1, region));
    return (1);
}

FT_TEST(test_game_world_registry_empty_world_fetch_is_not_found)
{
    game_world_registry value;
    game_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.fetch_world(1, world));
    return (1);
}

FT_TEST(test_game_world_registry_thread_safety_cycle_again)
{
    game_world_registry value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_world_registry_lock_cycle)
{
    game_world_registry value;
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}
