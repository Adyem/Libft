#include "terrain_generator.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "../Errno/errno.hpp"
#include "../RNG/rng.hpp"

static uint32_t terrain_generator_block_for_height(int32_t block_y) noexcept
{
    if (block_y > TERRAIN_GENERATOR_SURFACE_HEIGHT)
        return (GAME_VOXEL_AIR_BLOCK);
    if (block_y == TERRAIN_GENERATOR_SURFACE_HEIGHT)
        return (TERRAIN_GENERATOR_GRASS_BLOCK);
    if (block_y >= TERRAIN_GENERATOR_SURFACE_HEIGHT - 3)
        return (TERRAIN_GENERATOR_DIRT_BLOCK);
    return (TERRAIN_GENERATOR_STONE_BLOCK);
}

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    const char *seed_string) noexcept
{
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t block_id;
    int32_t error_code;
    int64_t seed_value;

    seed_value = rng_seed_value(seed_string);
    (void)seed_value;
    local_y = 0;
    while (local_y < GAME_VOXEL_CHUNK_HEIGHT)
    {
        block_id = terrain_generator_block_for_height(local_y);
        local_z = 0;
        while (local_z < GAME_VOXEL_CHUNK_DEPTH)
        {
            local_x = 0;
            while (local_x < GAME_VOXEL_CHUNK_WIDTH)
            {
                error_code = chunk.write_block(local_x, local_y, local_z,
                    block_id);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                local_x += 1;
            }
            local_z += 1;
        }
        local_y += 1;
    }
    chunk.clear_dirty();
    return (FT_ERR_SUCCESS);
}

#endif
