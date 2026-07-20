#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void data_catalog_copy_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.initialize(source);
    return ;
}

static void data_catalog_move_initialize_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.initialize(static_cast<game_data_catalog &&>(source));
    return ;
}

static void data_catalog_move_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_data_catalog_copy_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        data_catalog_copy_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_move_initialize_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        data_catalog_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_move_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        data_catalog_move_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_copy_destroyed_source_propagates)
{
    game_data_catalog source;
    game_data_catalog destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_data_catalog_move_destroyed_source_propagates)
{
    game_data_catalog source;
    game_data_catalog destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(
        FT_ERR_SUCCESS,
        destination.initialize(static_cast<game_data_catalog &&>(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_data_catalog_copy_into_initialized_destination_commits)
{
    game_data_catalog source;
    game_data_catalog destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(0, destination.get_recipes().size());
    return (1);
}

FT_TEST(test_game_data_catalog_move_into_initialized_destination_commits)
{
    game_data_catalog source;
    game_data_catalog destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(
        FT_ERR_SUCCESS,
        destination.initialize(static_cast<game_data_catalog &&>(source)));
    FT_ASSERT_EQ(0, destination.get_loadouts().size());
    return (1);
}

FT_TEST(test_game_data_catalog_self_copy_is_safe)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(value));
    return (1);
}

FT_TEST(test_game_data_catalog_self_move_is_safe_again)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}

FT_TEST(test_game_data_catalog_destructor_after_destroy_is_safe)
{
    game_data_catalog *value;

    value = new game_data_catalog();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
