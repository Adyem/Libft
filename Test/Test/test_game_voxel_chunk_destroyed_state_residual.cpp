#include "../../Modules/Game/game_voxel_chunk.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void chunk_destroyed_get_section_const(game_voxel_chunk &value)
{
    const game_voxel_chunk &constant_value = value;

    (void)constant_value.get_section(0);
    return ;
}

FT_TEST(test_game_voxel_chunk_destroyed_get_section_const_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_get_section_const));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_move_self_is_safe)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_get_error_str_is_valid)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_state_can_reinitialize_again)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_dirty());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_cleanup_is_idempotent_again)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_destructor_after_cleanup_is_safe_again)
{
    game_voxel_chunk *value;

    value = new game_voxel_chunk();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_reinitialized_write_is_clean)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.write_block(0, 0, 0, 5));
    FT_ASSERT_EQ(FT_TRUE, value.is_dirty());
    return (1);
}

FT_TEST(
    test_game_voxel_chunk_destroyed_reinitialized_read_returns_written_block)
{
    game_voxel_chunk value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.write_block(0, 0, 0, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(5, block_id);
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_reinitialized_clear_dirty_is_safe)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.clear_dirty();
    FT_ASSERT_EQ(FT_FALSE, value.is_dirty());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_error_remains_success)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_cleanup_can_repeat)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
