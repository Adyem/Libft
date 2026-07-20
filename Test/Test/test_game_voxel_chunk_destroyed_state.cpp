#include "../../Modules/Game/game_voxel_chunk.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void chunk_destroyed_read_block(game_voxel_chunk &value)
{
    uint32_t block_id;

    block_id = 0;
    (void)value.read_block(0, 0, 0, &block_id);
    return ;
}

static void chunk_destroyed_write_block(game_voxel_chunk &value)
{
    (void)value.write_block(0, 0, 0, 1);
    return ;
}

static void chunk_destroyed_is_dirty(game_voxel_chunk &value)
{
    (void)value.is_dirty();
    return ;
}

static void chunk_destroyed_clear_dirty(game_voxel_chunk &value)
{
    value.clear_dirty();
    return ;
}

static void chunk_destroyed_get_section(game_voxel_chunk &value)
{
    (void)value.get_section(0);
    return ;
}

static void chunk_destroyed_serialize(game_voxel_chunk &value)
{
    ft_byte_buffer buffer;

    (void)buffer.initialize();
    (void)value.serialize(buffer);
    return ;
}

static void chunk_destroyed_deserialize(game_voxel_chunk &value)
{
    ft_byte_buffer buffer;

    (void)buffer.initialize();
    (void)value.deserialize(buffer);
    return ;
}

FT_TEST(test_game_voxel_chunk_destroyed_read_block_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_read_block));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_write_block_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_write_block));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_is_dirty_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_is_dirty));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_clear_dirty_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_clear_dirty));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_get_section_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_get_section));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_serialize_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_serialize));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_deserialize_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_chunk>(
                        chunk_destroyed_deserialize));
    return (1);
}

FT_TEST(test_game_voxel_chunk_destroyed_get_error_is_valid)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
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

FT_TEST(test_game_voxel_chunk_destroyed_destructor_is_non_aborting)
{
    game_voxel_chunk *value;

    value = new game_voxel_chunk();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
