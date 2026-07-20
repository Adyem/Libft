#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_voxel_region_null_storage_path_reports_argument)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.set_region_storage_path(ft_nullptr));
    return (1);
}

FT_TEST(test_game_voxel_region_missing_file_load_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.load_region(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_null_chunk_output_reports_argument)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.load_chunk(0, 0, ft_nullptr));
    return (1);
}

FT_TEST(test_game_voxel_region_chunk_before_region_start_is_out_of_range)
{
    game_voxel_region value;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.load_chunk(-1, 0, &chunk_pointer));
    return (1);
}

FT_TEST(test_game_voxel_region_chunk_after_region_end_is_out_of_range)
{
    game_voxel_region value;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.load_chunk(GAME_VOXEL_REGION_CHUNKS_X, 0, &chunk_pointer));
    return (1);
}

FT_TEST(test_game_voxel_region_read_null_output_reports_argument)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.read_block(0, 0, 0, ft_nullptr));
    return (1);
}

FT_TEST(test_game_voxel_region_read_negative_height_reports_range)
{
    game_voxel_region value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.read_block(0, -1, 0, &block_id));
    return (1);
}

FT_TEST(test_game_voxel_region_read_height_boundary_reports_range)
{
    game_voxel_region value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.read_block(0, GAME_VOXEL_CHUNK_HEIGHT, 0, &block_id));
    return (1);
}

FT_TEST(test_game_voxel_region_write_outside_block_width_reports_range)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.write_block(GAME_VOXEL_REGION_BLOCK_WIDTH, 0, 0, 1));
    return (1);
}

FT_TEST(test_game_voxel_region_empty_block_reads_air)
{
    game_voxel_region value;
    uint32_t block_id;

    block_id = 99;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    return (1);
}

FT_TEST(test_game_voxel_region_empty_chunk_queries_are_false)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.has_chunk(0, 0));
    FT_ASSERT_EQ(FT_FALSE, value.is_chunk_loaded(0, 0));
    return (1);
}

