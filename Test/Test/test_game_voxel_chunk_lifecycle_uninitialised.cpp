#include "../../Modules/Game/game_voxel_chunk.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_voxel_chunk_read_block(game_voxel_chunk &value)
{
    uint32_t block_id = 0U;

    (void)value.read_block(0, 0, 0, &block_id);
    return ;
}

static void game_voxel_chunk_write_block(game_voxel_chunk &value)
{
    (void)value.write_block(0, 0, 0, 1U);
    return ;
}

static void game_voxel_chunk_is_dirty(game_voxel_chunk &value)
{
    (void)value.is_dirty();
    return ;
}

static void game_voxel_chunk_clear_dirty(game_voxel_chunk &value)
{
    value.clear_dirty();
    return ;
}

static void game_voxel_chunk_get_section(game_voxel_chunk &value)
{
    (void)value.get_section(0U);
    return ;
}

static void game_voxel_chunk_get_section_const(game_voxel_chunk &value)
{
    const game_voxel_chunk &constant_value = value;

    (void)constant_value.get_section(0U);
    return ;
}

static void game_voxel_chunk_serialize(game_voxel_chunk &value)
{
    ft_byte_buffer buffer;

    (void)value.serialize(buffer);
    return ;
}

static void game_voxel_chunk_deserialize(game_voxel_chunk &value)
{
    ft_byte_buffer buffer;

    (void)value.deserialize(buffer);
    return ;
}

static void game_voxel_chunk_get_error(game_voxel_chunk &value)
{
    (void)value.get_error();
    return ;
}

static void game_voxel_chunk_get_error_str(game_voxel_chunk &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_voxel_chunk_read_block_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_read_block));
    return (1);
}

FT_TEST(test_game_voxel_chunk_write_block_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_write_block));
    return (1);
}

FT_TEST(test_game_voxel_chunk_is_dirty_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_is_dirty));
    return (1);
}

FT_TEST(test_game_voxel_chunk_clear_dirty_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_clear_dirty));
    return (1);
}

FT_TEST(test_game_voxel_chunk_get_section_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_get_section));
    return (1);
}

FT_TEST(test_game_voxel_chunk_get_section_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_get_section_const));
    return (1);
}

FT_TEST(test_game_voxel_chunk_serialize_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_serialize));
    return (1);
}

FT_TEST(test_game_voxel_chunk_deserialize_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_deserialize));
    return (1);
}

FT_TEST(test_game_voxel_chunk_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_get_error));
    return (1);
}

FT_TEST(test_game_voxel_chunk_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_chunk>(
                        game_voxel_chunk_get_error_str));
    return (1);
}
