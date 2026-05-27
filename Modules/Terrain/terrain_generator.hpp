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
# define TERRAIN_GENERATOR_SHRUB_BLOCK 4U
# define TERRAIN_GENERATOR_OAK_LOG_BLOCK 5U
# define TERRAIN_GENERATOR_OAK_LEAVES_BLOCK 6U
# define TERRAIN_GENERATOR_CACTUS_BLOCK 7U
# define TERRAIN_BIOME_ZONE_WIDTH 128

enum terrain_biome
{
    TERRAIN_BIOME_PLAINS = 0,
    TERRAIN_BIOME_HILLS = 1,
    TERRAIN_BIOME_DESERT = 2,
    TERRAIN_BIOME_SNOW = 3,
    TERRAIN_BIOME_MOUNTAINS = 4
};

struct terrain_biome_profile
{
    int32_t surface_height;
    int32_t height_variation;
    int32_t topsoil_depth;
};

struct terrain_tree_template_block
{
    int32_t offset_x;
    int32_t offset_y;
    int32_t offset_z;
    uint32_t block_id;
};

struct terrain_tree_template
{
    const terrain_tree_template_block *blocks;
    uint32_t block_count;
};

terrain_biome terrain_get_biome(int32_t world_block_x, int32_t world_block_z,
    const char *seed_string = ft_nullptr) noexcept;
terrain_biome_profile terrain_get_biome_profile(terrain_biome biome) noexcept;
uint32_t terrain_surface_block_for_biome(terrain_biome biome) noexcept;
ft_bool terrain_biome_has_shrubs(terrain_biome biome) noexcept;
ft_bool terrain_biome_has_trees(terrain_biome biome) noexcept;
const terrain_tree_template &terrain_small_oak_tree_template(
    uint32_t variant_index = 0U) noexcept;
const terrain_tree_template &terrain_small_pine_tree_template(
    uint32_t variant_index = 0U) noexcept;
const terrain_tree_template &terrain_small_cactus_tree_template(
    uint32_t variant_index = 0U) noexcept;
const terrain_tree_template &terrain_large_oak_tree_template(
    uint32_t variant_index = 0U) noexcept;
const terrain_tree_template &terrain_large_pine_tree_template(
    uint32_t variant_index = 0U) noexcept;
const terrain_tree_template &terrain_small_oak_tree_template_variant(
    uint32_t variant_index) noexcept;
const terrain_tree_template &terrain_small_pine_tree_template_variant(
    uint32_t variant_index) noexcept;
const terrain_tree_template &terrain_small_cactus_tree_template_variant(
    uint32_t variant_index) noexcept;
const terrain_tree_template &terrain_large_oak_tree_template_variant(
    uint32_t variant_index) noexcept;
const terrain_tree_template &terrain_large_pine_tree_template_variant(
    uint32_t variant_index) noexcept;
const terrain_tree_template &terrain_tree_template_for_biome(
    terrain_biome biome) noexcept;
const terrain_tree_template &terrain_tree_template_for_biome(
    terrain_biome biome, uint64_t seed_value) noexcept;
ft_bool terrain_can_place_tree_template(game_voxel_chunk &chunk,
    int32_t local_origin_x, int32_t local_origin_y, int32_t local_origin_z,
    const terrain_tree_template &tree_template) noexcept;
int32_t terrain_place_tree_template(game_voxel_chunk &chunk,
    int32_t local_origin_x, int32_t local_origin_y, int32_t local_origin_z,
    const terrain_tree_template &tree_template) noexcept;

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    const char *seed_string = ft_nullptr) noexcept;
int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    int32_t world_block_origin_x, int32_t world_block_origin_z,
    const char *seed_string = ft_nullptr) noexcept;

#endif

#endif
