#include "../../Modules/Game/game_voxel_chunk.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void voxel_chunk_initialize_twice(game_voxel_chunk &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_voxel_chunk_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        voxel_chunk_initialize_twice));
    return (1);
}

FT_TEST(test_game_voxel_chunk_initialize_succeeds_clean)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_dirty());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroy_twice_is_safe)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_reinitialize_after_destroy)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_dirty());
    return (1);
}

FT_TEST(test_game_voxel_chunk_self_move_is_safe)
{
    game_voxel_chunk value;
    uint32_t block_id;

    block_id = 99;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    return (1);
}

FT_TEST(test_game_voxel_chunk_clear_dirty_is_idempotent)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.clear_dirty();
    value.clear_dirty();
    FT_ASSERT_EQ(FT_FALSE, value.is_dirty());
    return (1);
}
