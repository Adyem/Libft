#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void world_region_copy_uninitialised(game_world_region &value)
{
    game_world_region source;

    (void)value.initialize(source);
    return ;
}

static void world_region_move_initialize_uninitialised(game_world_region &value)
{
    game_world_region source;

    (void)value.initialize(static_cast<game_world_region &&>(source));
    return ;
}

static void world_region_move_uninitialised(game_world_region &value)
{
    game_world_region source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_world_region_copy_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_region>(
                        world_region_copy_uninitialised));
    return (1);
}

FT_TEST(test_game_world_region_move_initialize_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_region>(
                        world_region_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_world_region_move_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_region>(
                        world_region_move_uninitialised));
    return (1);
}

FT_TEST(test_game_world_region_copy_destroyed_source_propagates)
{
    game_world_region source;
    game_world_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_world_region_move_destroyed_source_propagates)
{
    game_world_region source;
    game_world_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(
        FT_ERR_SUCCESS,
        destination.initialize(static_cast<game_world_region &&>(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_world_region_copy_into_initialized_destination_commits)
{
    game_world_region source;
    game_world_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_world_id(22);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    destination.set_world_id(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(22, destination.get_world_id());
    return (1);
}

FT_TEST(test_game_world_region_move_into_initialized_destination_commits)
{
    game_world_region source;
    game_world_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_world_id(23);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(
        FT_ERR_SUCCESS,
        destination.initialize(static_cast<game_world_region &&>(source)));
    FT_ASSERT_EQ(23, destination.get_world_id());
    return (1);
}

FT_TEST(test_game_world_region_self_copy_is_safe)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(value));
    return (1);
}

FT_TEST(test_game_world_region_self_move_is_safe_again)
{
    game_world_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}

FT_TEST(test_game_world_region_destructor_after_destroy_is_safe)
{
    game_world_region *value;

    value = new game_world_region();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
