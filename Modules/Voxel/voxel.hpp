#ifndef VOXEL_HPP
# define VOXEL_HPP

#ifdef GAME_USE_VOXEL_REGION_BACKEND

# include "../Game/game_voxel_chunk.hpp"
# include "../Basic/class_nullptr.hpp"
# include <stdint.h>

# define TERRAIN_GENERATOR_SURFACE_HEIGHT 76
# define TERRAIN_GENERATOR_SEA_LEVEL      72
# define TERRAIN_GENERATOR_WATER_BLOCK 8U
# define TERRAIN_GENERATOR_GRASS_BLOCK 1U
# define TERRAIN_GENERATOR_DIRT_BLOCK 2U
# define TERRAIN_GENERATOR_STONE_BLOCK 3U
# define TERRAIN_GENERATOR_SHRUB_BLOCK 4U
# define TERRAIN_GENERATOR_OAK_LOG_BLOCK 5U
# define TERRAIN_GENERATOR_OAK_LEAVES_BLOCK 6U
# define TERRAIN_GENERATOR_CACTUS_BLOCK 7U
# define TERRAIN_GENERATOR_WATER_BLOCK 8U
# define TERRAIN_GENERATOR_BEDROCK_BLOCK 9U
# define TERRAIN_GENERATOR_SAND_BLOCK 10U
# define TERRAIN_GENERATOR_SNOW_BLOCK 11U
# define TERRAIN_GENERATOR_PERMAFROST_BLOCK 12U
# define TERRAIN_GENERATOR_CANYON_ROCK_BLOCK 13U
# define TERRAIN_GENERATOR_SLATE_BLOCK 14U
# define TERRAIN_GENERATOR_MOSS_ROCK_BLOCK 15U
# define TERRAIN_BIOME_ZONE_WIDTH 128
# define TERRAIN_MAX_CUSTOM_BIOMES 16U
# define TERRAIN_MAX_FEATURE_RULES 16U

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

struct terrain_block_metadata
{
    ft_bool solid;
    ft_bool transparent;
    ft_bool liquid;
    ft_bool replaceable;
    ft_bool light_emitting;
    ft_bool occludes_faces;
    uint32_t hardness;
    ft_bool breakable;
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

struct terrain_biome_definition
{
    terrain_biome_profile profile;
    uint32_t surface_block_id;
    uint32_t subsurface_block_id;
    uint32_t deep_block_id;
    ft_bool allow_shrubs;
    ft_bool allow_trees;
    uint32_t shrub_chance_percent;
    uint32_t tree_chance_percent;
    const terrain_tree_template *tree_template;
};

struct terrain_feature_rule
{
    const terrain_tree_template *template_data;
    int32_t biome_index;
    uint32_t chance_percent;
    int32_t minimum_height;
    int32_t maximum_height;
    ft_bool requires_dry_land;
};

typedef uint32_t (*terrain_biome_selector)(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z, uint32_t biome_count,
    void *user_data) noexcept;

struct terrain_generation_config
{
    int32_t sea_level;
    int32_t large_noise_scale;
    int32_t detail_noise_scale;
    int32_t detail_noise_percent;
    uint32_t water_chance_percent;
    uint32_t biome_count;
    terrain_biome_definition biomes[TERRAIN_MAX_CUSTOM_BIOMES];
    terrain_biome_selector biome_selector;
    void *biome_selector_user_data;
    uint32_t feature_count;
    terrain_feature_rule features[TERRAIN_MAX_FEATURE_RULES];
};

terrain_generation_config terrain_default_generation_config() noexcept;
ft_bool terrain_generation_config_is_valid(
    const terrain_generation_config &config) noexcept;
uint32_t terrain_select_biome(const terrain_generation_config &config,
    uint64_t seed_value, int32_t world_block_x, int32_t world_block_z) noexcept;
uint32_t terrain_get_biome_index(const terrain_generation_config &config,
    int32_t world_block_x, int32_t world_block_z,
    const char *seed_string = ft_nullptr) noexcept;

terrain_biome terrain_get_biome(int32_t world_block_x, int32_t world_block_z,
    const char *seed_string = ft_nullptr) noexcept;
terrain_biome_profile terrain_get_biome_profile(terrain_biome biome) noexcept;
const terrain_block_metadata &terrain_get_block_metadata(
    uint32_t block_id) noexcept;
ft_bool terrain_block_is_known(uint32_t block_id) noexcept;
ft_bool terrain_block_is_solid(uint32_t block_id) noexcept;
ft_bool terrain_block_is_transparent(uint32_t block_id) noexcept;
ft_bool terrain_block_is_liquid(uint32_t block_id) noexcept;
ft_bool terrain_block_is_replaceable(uint32_t block_id) noexcept;
ft_bool terrain_block_emits_light(uint32_t block_id) noexcept;
ft_bool terrain_block_occludes_faces(uint32_t block_id) noexcept;
uint32_t terrain_block_hardness(uint32_t block_id) noexcept;
ft_bool terrain_block_is_breakable(uint32_t block_id) noexcept;
uint32_t terrain_surface_block_for_biome(terrain_biome biome) noexcept;
uint32_t terrain_subsurface_block_for_biome(terrain_biome biome) noexcept;
uint32_t terrain_deep_block_for_biome(terrain_biome biome) noexcept;
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
int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    int32_t world_block_origin_x, int32_t world_block_origin_z,
    const char *seed_string, const terrain_generation_config &config) noexcept;

#endif

#endif
