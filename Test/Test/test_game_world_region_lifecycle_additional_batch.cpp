#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void world_region_initialize_twice(game_world_region &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void world_region_move_uninitialised(game_world_region &value)
{
    game_world_region source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_world_region_initialize_twice_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_region>(
                        world_region_initialize_twice));
    return (1);
}

FT_TEST(test_game_world_region_move_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_region>(
                        world_region_move_uninitialised));
    return (1);
}

FT_TEST(test_game_world_region_default_initialize_has_empty_regions)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_region_ids().size());
    return (1);
}

FT_TEST(test_game_world_region_destroy_twice_is_safe_again)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_world_region_reinitialize_after_destroy_again)
{
    game_world_region value;
    ft_vector<int32_t> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_region_ids().size());
    return (1);
}

FT_TEST(test_game_world_region_set_world_id_preserves_value)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_world_id(17);
    FT_ASSERT_EQ(17, value.get_world_id());
    return (1);
}

FT_TEST(test_game_world_region_set_empty_region_ids)
{
    game_world_region value;
    ft_vector<int32_t> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    value.set_region_ids(region_ids);
    FT_ASSERT_EQ(0, value.get_region_ids().size());
    return (1);
}

FT_TEST(test_game_world_region_thread_safety_cycle_again)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_world_region_error_after_initialization)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_world_region_lock_cycle)
{
    game_world_region value;
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
