#ifndef TERRAIN_GENERATOR_HPP
# define TERRAIN_GENERATOR_HPP

#ifdef GAME_USE_VOXEL_REGION_BACKEND

# include "../Game/game_voxel_chunk.hpp"
# include "../CPP_class/class_nullptr.hpp"
# include <stdint.h>

# define TERRAIN_GENERATOR_SURFACE_HEIGHT 76
# define TERRAIN_GENERATOR_GRASS_BLOCK 1U
# define TERRAIN_GENERATOR_DIRT_BLOCK 2U
# define TERRAIN_GENERATOR_STONE_BLOCK 3U

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    const char *seed_string = ft_nullptr) noexcept;

#endif

#endif
