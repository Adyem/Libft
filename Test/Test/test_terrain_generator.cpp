#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "../../Modules/Terrain/terrain_generator.hpp"

FT_TEST(test_terrain_generate_chunk_generates_default_surface_chunk)
{
    game_voxel_chunk chunk;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk,
        "terrain-test-seed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_STONE_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0,
        TERRAIN_GENERATOR_SURFACE_HEIGHT - 1, 0, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_DIRT_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0,
        TERRAIN_GENERATOR_SURFACE_HEIGHT, 0, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_GRASS_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0,
        TERRAIN_GENERATOR_SURFACE_HEIGHT + 1, 0, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_FALSE, chunk.is_dirty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generate_chunk_accepts_random_seed_placeholder)
{
    game_voxel_chunk chunk;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

#endif
