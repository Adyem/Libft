#include "../../Modules/Game/game_voxel_chunk.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_voxel_chunk_read_negative_x_reports_range)
{
    game_voxel_chunk value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.read_block(-1, 0, 0, &block_id));
    return (1);
}

FT_TEST(test_game_voxel_chunk_read_width_boundary_reports_range)
{
    game_voxel_chunk value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE,
                 value.read_block(GAME_VOXEL_CHUNK_WIDTH, 0, 0, &block_id));
    return (1);
}

FT_TEST(test_game_voxel_chunk_read_height_boundary_reports_range)
{
    game_voxel_chunk value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE,
                 value.read_block(0, GAME_VOXEL_CHUNK_HEIGHT, 0, &block_id));
    return (1);
}

FT_TEST(test_game_voxel_chunk_read_null_output_reports_argument)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
                 value.read_block(0, 0, 0, ft_nullptr));
    return (1);
}

FT_TEST(test_game_voxel_chunk_write_negative_z_reports_range)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.write_block(0, 0, -1, 3));
    return (1);
}

FT_TEST(test_game_voxel_chunk_write_max_valid_coordinate_succeeds)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.write_block(GAME_VOXEL_CHUNK_WIDTH - 1,
                                   GAME_VOXEL_CHUNK_HEIGHT - 1,
                                   GAME_VOXEL_CHUNK_DEPTH - 1, 7));
    return (1);
}

FT_TEST(test_game_voxel_chunk_write_height_boundary_reports_range)
{
    game_voxel_chunk value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE,
                 value.write_block(0, GAME_VOXEL_CHUNK_HEIGHT, 0, 7));
    return (1);
}

FT_TEST(test_game_voxel_chunk_empty_serialization_can_be_read)
{
    game_voxel_chunk value;
    game_voxel_chunk restored_value;
    ft_byte_buffer buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, restored_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.serialize(buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, restored_value.deserialize(buffer));
    FT_ASSERT_EQ(FT_FALSE, restored_value.is_dirty());
    return (1);
}

FT_TEST(test_game_voxel_chunk_empty_deserialization_reports_io)
{
    game_voxel_chunk value;
    ft_byte_buffer buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_IO, value.deserialize(buffer));
    return (1);
}

FT_TEST(test_game_voxel_chunk_invalid_serialization_header_reports_argument)
{
    game_voxel_chunk value;
    ft_byte_buffer buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u32_le(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u32_le(0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.deserialize(buffer));
    return (1);
}
