#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(
    test_game_voxel_region_world_coordinate_rounds_down_at_positive_boundary)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(513, 511, "."));
    FT_ASSERT_EQ(512, value.get_region_start_x());
    FT_ASSERT_EQ(0, value.get_region_start_z());
    return (1);
}

FT_TEST(
    test_game_voxel_region_world_coordinate_rounds_down_at_negative_boundary)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(-1, -513, "."));
    FT_ASSERT_EQ(-512, value.get_region_start_x());
    FT_ASSERT_EQ(-1024, value.get_region_start_z());
    return (1);
}

FT_TEST(test_game_voxel_region_write_read_last_block_in_region)
{
    game_voxel_region value;
    uint32_t block_id;

    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.write_block(GAME_VOXEL_REGION_BLOCK_WIDTH - 1, 0,
                                   GAME_VOXEL_REGION_BLOCK_WIDTH - 1, 27));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.read_block(GAME_VOXEL_REGION_BLOCK_WIDTH - 1, 0,
                                  GAME_VOXEL_REGION_BLOCK_WIDTH - 1,
                                  &block_id));
    FT_ASSERT_EQ(27, block_id);
    return (1);
}

FT_TEST(test_game_voxel_region_negative_x_is_out_of_range)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.write_block(-1, 0, 0, 1));
    return (1);
}

FT_TEST(test_game_voxel_region_negative_z_is_out_of_range)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.write_block(0, 0, -1, 1));
    return (1);
}

FT_TEST(test_game_voxel_region_last_chunk_coordinates_are_loadable)
{
    game_voxel_region value;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.load_chunk(GAME_VOXEL_REGION_CHUNKS_X - 1,
                                  GAME_VOXEL_REGION_CHUNKS_Z - 1,
                                  &chunk_pointer));
    FT_ASSERT(chunk_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, value.has_chunk(GAME_VOXEL_REGION_CHUNKS_X - 1,
                                          GAME_VOXEL_REGION_CHUNKS_Z - 1));
    return (1);
}

FT_TEST(test_game_voxel_region_chunk_after_z_boundary_is_out_of_range)
{
    game_voxel_region value;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(
        FT_ERR_OUT_OF_RANGE,
        value.load_chunk(0, GAME_VOXEL_REGION_CHUNKS_Z, &chunk_pointer));
    FT_ASSERT_EQ(ft_nullptr, chunk_pointer);
    return (1);
}

FT_TEST(test_game_voxel_region_save_and_load_chunk_preserves_block)
{
    game_voxel_region value;
    game_voxel_chunk source_chunk;
    game_voxel_chunk *loaded_chunk;
    uint32_t block_id;

    loaded_chunk = ft_nullptr;
    block_id = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_chunk.write_block(1, 2, 3, 99));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.save_chunk(0, 0, source_chunk));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.load_chunk(0, 0, &loaded_chunk));
    FT_ASSERT(loaded_chunk != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk->read_block(1, 2, 3, &block_id));
    FT_ASSERT_EQ(99, block_id);
    return (1);
}

FT_TEST(test_game_voxel_region_save_chunk_outside_region_is_rejected)
{
    game_voxel_region value;
    game_voxel_chunk source_chunk;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE,
                 value.save_chunk(GAME_VOXEL_REGION_CHUNKS_X, 0, source_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_non_null_storage_path_is_retained)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.set_region_storage_path("missing-storage"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_voxel_region_load_missing_region_resets_loaded_state)
{
    game_voxel_region value;
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.set_region_storage_path("missing-storage"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.load_chunk(0, 0, &chunk_pointer));
    FT_ASSERT(chunk_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.load_region(256, 256));
    FT_ASSERT_EQ(FT_FALSE, value.has_chunk(0, 0));
    return (1);
}
