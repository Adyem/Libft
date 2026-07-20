#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void voxel_region_move_from_uninitialised(game_voxel_region &value)
{
    game_voxel_region source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_voxel_region_move_copies_region_coordinates)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(513, -1, "."));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(512, destination.get_region_start_x());
    FT_ASSERT_EQ(-512, destination.get_region_start_z());
    return (1);
}

FT_TEST(test_game_voxel_region_move_replaces_initialized_destination)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(512, 512, "."));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(512, destination.get_region_start_x());
    FT_ASSERT_EQ(512, destination.get_region_start_z());
    return (1);
}

FT_TEST(
    test_game_voxel_region_move_from_destroyed_source_leaves_destination_destroyed)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_move_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        voxel_region_move_from_uninitialised));
    return (1);
}

FT_TEST(test_game_voxel_region_move_self_preserves_coordinates)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(-1, -1, "."));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(-512, value.get_region_start_x());
    FT_ASSERT_EQ(-512, value.get_region_start_z());
    return (1);
}

FT_TEST(test_game_voxel_region_move_source_can_be_reinitialized)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(256, 256, "."));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(0, source.get_region_start_x());
    FT_ASSERT_EQ(0, source.get_region_start_z());
    return (1);
}

FT_TEST(
    test_game_voxel_region_move_destination_can_reinitialize_after_destroyed_source)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(0, destination.get_region_start_x());
    return (1);
}

FT_TEST(test_game_voxel_region_move_transfers_loaded_chunk)
{
    game_voxel_region source;
    game_voxel_region destination;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.load_chunk(0, 0, &chunk_pointer));
    FT_ASSERT(chunk_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_TRUE, destination.has_chunk(0, 0));
    FT_ASSERT_EQ(FT_FALSE, source.has_chunk(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_move_transfers_dirty_chunk_state)
{
    game_voxel_region source;
    game_voxel_region destination;
    game_voxel_chunk source_chunk;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_chunk.write_block(0, 0, 0, 12));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.save_chunk(0, 0, source_chunk));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_TRUE, destination.has_chunk(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_move_cleanup_is_idempotent)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_move_preserves_success_error_state)
{
    game_voxel_region source;
    game_voxel_region destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}
