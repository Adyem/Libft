#include <stdint.h>
#include "voxel.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "voxel_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Game/game_voxel_chunk.hpp"

static const int32_t TERRAIN_HEIGHTMAP_LARGE_SCALE = 32;
static const int32_t TERRAIN_HEIGHTMAP_DETAIL_SCALE = 8;
static const int32_t TERRAIN_BIOME_ZONE_BLEND_WIDTH = 16;
static const int32_t TERRAIN_HEIGHTMAP_SMOOTH_RADIUS = 1;
static const uint64_t TERRAIN_FEATURE_SHRUB_SALT = UINT64_C(0x2D9C1F4E8B3A6071);
static const int32_t TERRAIN_FEATURE_SHRUB_HEIGHT_OFFSET = 1;
static const uint64_t TERRAIN_FEATURE_SHRUB_THRESHOLD = 6U;
static const uint64_t TERRAIN_FEATURE_TREE_SALT = UINT64_C(0x4F1E2D3C5B6A7980);
static const uint64_t TERRAIN_FEATURE_WATER_SALT = UINT64_C(0x9182736455463728);
static const uint64_t TERRAIN_CAVE_PRIMARY_SALT = UINT64_C(0x7C3A91E2D4B8560F);
static const uint64_t TERRAIN_CAVE_DETAIL_SALT = UINT64_C(0x1D6F80B3C9274A55);
static const int32_t TERRAIN_CAVE_PRIMARY_SCALE = 24;
static const int32_t TERRAIN_CAVE_DETAIL_SCALE = 9;
static const int32_t TERRAIN_CAVE_MIN_Y = 8;
static const int32_t TERRAIN_CAVE_SURFACE_MARGIN = 7;
static const int32_t TERRAIN_COLUMN_CACHE_COUNT =
    GAME_VOXEL_CHUNK_WIDTH * GAME_VOXEL_CHUNK_DEPTH;

struct terrain_column_cache
{
    uint32_t biome;
    terrain_biome_profile biome_profile;
    int32_t column_height;
    uint32_t surface_block_id;
    uint32_t subsurface_block_id;
    uint32_t deep_block_id;
    ft_bool can_place_shrubs;
    ft_bool can_place_trees;
};

static int32_t terrain_column_height(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z,
    const terrain_biome_profile &biome_profile,
    const terrain_generation_config &config) noexcept
{
    double large_noise;
    double detail_noise;
    double total_noise;
    int32_t surface_height;
    int32_t variation;

    large_noise = terrain_value_noise(seed_value, world_block_x, world_block_z,
        config.large_noise_scale > 0 ? config.large_noise_scale : TERRAIN_HEIGHTMAP_LARGE_SCALE);
    detail_noise = terrain_value_noise(seed_value ^ UINT64_C(0xA5A5A5A5A5A5A5A5),
        world_block_x, world_block_z,
        config.detail_noise_scale > 0 ? config.detail_noise_scale : TERRAIN_HEIGHTMAP_DETAIL_SCALE);
    variation = biome_profile.height_variation;
    total_noise = (large_noise * static_cast<double>(variation))
        + (detail_noise * static_cast<double>(variation)
            * static_cast<double>(config.detail_noise_percent) / 100.0);
    surface_height = biome_profile.surface_height
        + static_cast<int32_t>(total_noise);
    return (surface_height);
}

static double terrain_zone_blend_factor(int32_t local_coordinate) noexcept
{
    double blend_factor;

    blend_factor = static_cast<double>(local_coordinate)
        / static_cast<double>(TERRAIN_BIOME_ZONE_BLEND_WIDTH);
    if (blend_factor < 0.0)
        blend_factor = 0.0;
    if (blend_factor > 1.0)
        blend_factor = 1.0;
    return (blend_factor);
}

static int32_t terrain_blend_height(int32_t left_height, int32_t right_height,
    double factor) noexcept
{
    return (static_cast<int32_t>(terrain_lerp(
        static_cast<double>(left_height),
        static_cast<double>(right_height), factor)));
}

static int32_t terrain_smooth_biome_height(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z,
    const terrain_biome_profile &biome_profile,
    const terrain_generation_config &config) noexcept
{
    int32_t biome_zone_x;
    int32_t biome_zone_z;
    int32_t zone_local_x;
    int32_t zone_local_z;
    int32_t height;
    int32_t neighbor_x;
    int32_t neighbor_z;
    uint32_t neighbor_biome;
    terrain_biome_profile neighbor_profile;
    int32_t neighbor_height;
    double blend_factor;

    biome_zone_x = terrain_floor_div(world_block_x, TERRAIN_BIOME_ZONE_WIDTH);
    biome_zone_z = terrain_floor_div(world_block_z, TERRAIN_BIOME_ZONE_WIDTH);
    zone_local_x = world_block_x - (biome_zone_x * TERRAIN_BIOME_ZONE_WIDTH);
    zone_local_z = world_block_z - (biome_zone_z * TERRAIN_BIOME_ZONE_WIDTH);
    height = terrain_column_height(seed_value, world_block_x, world_block_z,
        biome_profile, config);
    if (zone_local_x < TERRAIN_BIOME_ZONE_BLEND_WIDTH)
    {
        neighbor_x = world_block_x - TERRAIN_BIOME_ZONE_WIDTH;
        neighbor_biome = terrain_select_biome(config, seed_value, neighbor_x,
            world_block_z);
        neighbor_profile = config.biomes[neighbor_biome].profile;
        neighbor_height = terrain_column_height(seed_value, neighbor_x,
            world_block_z, neighbor_profile, config);
        blend_factor = terrain_zone_blend_factor(zone_local_x);
        height = terrain_blend_height(neighbor_height, height, blend_factor);
    }
    else if (zone_local_x >= (TERRAIN_BIOME_ZONE_WIDTH
            - TERRAIN_BIOME_ZONE_BLEND_WIDTH))
    {
        neighbor_x = world_block_x + TERRAIN_BIOME_ZONE_WIDTH;
        neighbor_biome = terrain_select_biome(config, seed_value, neighbor_x,
            world_block_z);
        neighbor_profile = config.biomes[neighbor_biome].profile;
        neighbor_height = terrain_column_height(seed_value, neighbor_x,
            world_block_z, neighbor_profile, config);
        blend_factor = terrain_zone_blend_factor(
            (TERRAIN_BIOME_ZONE_WIDTH - 1) - zone_local_x);
        height = terrain_blend_height(height, neighbor_height, blend_factor);
    }
    if (zone_local_z < TERRAIN_BIOME_ZONE_BLEND_WIDTH)
    {
        neighbor_z = world_block_z - TERRAIN_BIOME_ZONE_WIDTH;
        neighbor_biome = terrain_select_biome(config, seed_value, world_block_x,
            neighbor_z);
        neighbor_profile = config.biomes[neighbor_biome].profile;
        neighbor_height = terrain_column_height(seed_value, world_block_x,
            neighbor_z, neighbor_profile, config);
        blend_factor = terrain_zone_blend_factor(zone_local_z);
        height = terrain_blend_height(neighbor_height, height, blend_factor);
    }
    else if (zone_local_z >= (TERRAIN_BIOME_ZONE_WIDTH
            - TERRAIN_BIOME_ZONE_BLEND_WIDTH))
    {
        neighbor_z = world_block_z + TERRAIN_BIOME_ZONE_WIDTH;
        neighbor_biome = terrain_select_biome(config, seed_value, world_block_x,
            neighbor_z);
        neighbor_profile = config.biomes[neighbor_biome].profile;
        neighbor_height = terrain_column_height(seed_value, world_block_x,
            neighbor_z, neighbor_profile, config);
        blend_factor = terrain_zone_blend_factor(
            (TERRAIN_BIOME_ZONE_WIDTH - 1) - zone_local_z);
        height = terrain_blend_height(height, neighbor_height, blend_factor);
    }
    return (height);
}

static int32_t terrain_sample_height(uint64_t seed_value, int32_t world_block_x,
    int32_t world_block_z, const terrain_generation_config &config) noexcept
{
    uint32_t biome;
    terrain_biome_profile biome_profile;
    int32_t minimum_height;
    int32_t maximum_height;
    int32_t height;

    biome = terrain_select_biome(config, seed_value, world_block_x, world_block_z);
    biome_profile = config.biomes[biome].profile;
    height = terrain_smooth_biome_height(seed_value, world_block_x,
        world_block_z, biome_profile, config);
    minimum_height = biome_profile.surface_height
        - biome_profile.height_variation
        - (biome_profile.height_variation / 2);
    maximum_height = biome_profile.surface_height
        + biome_profile.height_variation
        + (biome_profile.height_variation / 2);
    if (height < minimum_height)
        return (minimum_height);
    if (height > maximum_height)
        return (maximum_height);
    return (height);
}

static int32_t terrain_clamp_height_to_profile(int32_t height,
    const terrain_biome_profile &biome_profile) noexcept
{
    int32_t minimum_height;
    int32_t maximum_height;

    minimum_height = biome_profile.surface_height
        - biome_profile.height_variation
        - (biome_profile.height_variation / 2);
    maximum_height = biome_profile.surface_height
        + biome_profile.height_variation
        + (biome_profile.height_variation / 2);
    if (height < minimum_height)
        return (minimum_height);
    if (height > maximum_height)
        return (maximum_height);
    return (height);
}

static int32_t terrain_smooth_heightfield(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z,
    const terrain_biome_profile &biome_profile,
    const terrain_generation_config &config) noexcept
{
    int32_t offset_x;
    int32_t offset_z;
    int32_t sample_count;
    int32_t weighted_height;
    int32_t sample_height;
    int32_t sample_weight;

    offset_z = -TERRAIN_HEIGHTMAP_SMOOTH_RADIUS;
    sample_count = 0;
    weighted_height = 0;
    while (offset_z <= TERRAIN_HEIGHTMAP_SMOOTH_RADIUS)
    {
        offset_x = -TERRAIN_HEIGHTMAP_SMOOTH_RADIUS;
        while (offset_x <= TERRAIN_HEIGHTMAP_SMOOTH_RADIUS)
        {
            sample_height = terrain_sample_height(seed_value,
                world_block_x + offset_x, world_block_z + offset_z, config);
            if (offset_x == 0 && offset_z == 0)
                sample_weight = 4;
            else if (offset_x == 0 || offset_z == 0)
                sample_weight = 2;
            else
                sample_weight = 1;
            weighted_height += sample_height * sample_weight;
            sample_count += sample_weight;
            offset_x += 1;
        }
        offset_z += 1;
    }
    if (sample_count <= 0)
        return (terrain_clamp_height_to_profile(terrain_sample_height(
            seed_value, world_block_x, world_block_z, config), biome_profile));
    return (terrain_clamp_height_to_profile(weighted_height / sample_count,
        biome_profile));
}

static ft_bool terrain_should_place_feature(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z, uint64_t salt,
    uint64_t threshold) noexcept
{
    uint64_t feature_seed;

    feature_seed = terrain_feature_seed(seed_value, world_block_x,
        world_block_z, salt);
    if ((feature_seed % 100U) < threshold)
        return (FT_TRUE);
    return (FT_FALSE);
}

static double terrain_cave_noise(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_y, int32_t world_block_z,
    int32_t scale, uint64_t salt) noexcept
{
    uint64_t layer_seed;
    int32_t layer_y;

    layer_y = terrain_floor_div(world_block_y, scale);
    layer_seed = seed_value ^ salt
        ^ (static_cast<uint64_t>(layer_y) * UINT64_C(0x9E3779B97F4A7C15));
    return (terrain_value_noise(layer_seed,
        world_block_x + (world_block_y * 13),
        world_block_z - (world_block_y * 7), scale));
}

static ft_bool terrain_should_carve_cave(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_y, int32_t world_block_z,
    int32_t surface_height) noexcept
{
    double primary_noise;
    double detail_noise;

    if (world_block_y < TERRAIN_CAVE_MIN_Y)
        return (FT_FALSE);
    if (world_block_y >= surface_height - TERRAIN_CAVE_SURFACE_MARGIN)
        return (FT_FALSE);
    primary_noise = terrain_cave_noise(seed_value, world_block_x,
        world_block_y, world_block_z, TERRAIN_CAVE_PRIMARY_SCALE,
        TERRAIN_CAVE_PRIMARY_SALT);
    detail_noise = terrain_cave_noise(seed_value, world_block_x,
        world_block_y, world_block_z, TERRAIN_CAVE_DETAIL_SCALE,
        TERRAIN_CAVE_DETAIL_SALT);
    if (primary_noise > 0.34 && detail_noise > -0.12)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    const char *seed_string) noexcept
{
    return (terrain_generate_chunk(chunk, 0, 0, seed_string));
}

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    int32_t world_block_origin_x, int32_t world_block_origin_z,
    const char *seed_string) noexcept
{
    terrain_generation_config config = terrain_default_generation_config();
    return (terrain_generate_chunk(chunk, world_block_origin_x,
        world_block_origin_z, seed_string, config));
}

int32_t terrain_generate_chunk(game_voxel_chunk &chunk,
    int32_t world_block_origin_x, int32_t world_block_origin_z,
    const char *seed_string, const terrain_generation_config &config) noexcept
{
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t block_id;
    int32_t error_code;
    int32_t column_height;
    int32_t world_block_x;
    int32_t world_block_z;
    uint32_t biome;
    terrain_biome_profile biome_profile;
    uint32_t surface_block_id;
    uint32_t subsurface_block_id;
    uint32_t deep_block_id;
    uint64_t seed_value;
    ft_bool place_shrub;
    const terrain_tree_template *tree_template;
    uint64_t tree_feature_seed;
    uint32_t feature_index;
    uint64_t feature_seed;
    const terrain_feature_rule *feature_rule;
    terrain_column_cache column_cache[TERRAIN_COLUMN_CACHE_COUNT];
    int32_t column_index;

    if (terrain_generation_config_is_valid(config) == FT_FALSE)
        return (FT_ERR_INVALID_ARGUMENT);

    seed_value = terrain_seed_value(seed_string);
    local_z = 0;
    while (local_z < GAME_VOXEL_CHUNK_DEPTH)
    {
        world_block_z = world_block_origin_z + local_z;
        local_x = 0;
        while (local_x < GAME_VOXEL_CHUNK_WIDTH)
        {
            column_index = (local_z * GAME_VOXEL_CHUNK_WIDTH) + local_x;
            world_block_x = world_block_origin_x + local_x;
            column_cache[column_index].biome = terrain_select_biome(config,
                seed_value, world_block_x, world_block_z);
            column_cache[column_index].biome_profile
                = config.biomes[column_cache[column_index].biome].profile;
            column_cache[column_index].column_height
                = terrain_sample_height(seed_value, world_block_x,
                    world_block_z, config);
            column_cache[column_index].surface_block_id =
                config.biomes[column_cache[column_index].biome].surface_block_id;
            column_cache[column_index].subsurface_block_id =
                config.biomes[column_cache[column_index].biome].subsurface_block_id;
            column_cache[column_index].deep_block_id =
                config.biomes[column_cache[column_index].biome].deep_block_id;
            column_cache[column_index].can_place_shrubs
                = config.biomes[column_cache[column_index].biome].allow_shrubs;
            column_cache[column_index].can_place_trees
                = config.biomes[column_cache[column_index].biome].allow_trees;
            local_x += 1;
        }
        local_z += 1;
    }
    local_z = 0;
    while (local_z < GAME_VOXEL_CHUNK_DEPTH)
    {
        world_block_z = world_block_origin_z + local_z;
        local_x = 0;
        while (local_x < GAME_VOXEL_CHUNK_WIDTH)
        {
            column_index = (local_z * GAME_VOXEL_CHUNK_WIDTH) + local_x;
            world_block_x = world_block_origin_x + local_x;
            biome = column_cache[column_index].biome;
            biome_profile = column_cache[column_index].biome_profile;
            surface_block_id = column_cache[column_index].surface_block_id;
            subsurface_block_id = column_cache[column_index].subsurface_block_id;
            deep_block_id = column_cache[column_index].deep_block_id;
            place_shrub = column_cache[column_index].can_place_shrubs;
            column_height = terrain_smooth_heightfield(seed_value,
                world_block_x, world_block_z, biome_profile, config);
            column_cache[column_index].column_height = column_height;
            if (column_height < 0)
                column_height = 0;
            if (column_height >= GAME_VOXEL_CHUNK_HEIGHT)
                column_height = GAME_VOXEL_CHUNK_HEIGHT - 1;
            local_y = 0;
            while (local_y <= column_height)
            {
                if (terrain_should_carve_cave(seed_value, world_block_x,
                        local_y, world_block_z, column_height) == FT_TRUE)
                {
                    local_y += 1;
                    continue ;
                }
                if (local_y == column_height)
                    block_id = surface_block_id;
                else if (local_y >= column_height - biome_profile.topsoil_depth)
                    block_id = subsurface_block_id;
                else
                    block_id = deep_block_id;
                error_code = chunk.write_block(local_x, local_y, local_z,
                    block_id);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                local_y += 1;
            }
            if (column_height + TERRAIN_FEATURE_SHRUB_HEIGHT_OFFSET
                < GAME_VOXEL_CHUNK_HEIGHT
                && place_shrub == FT_TRUE
                && terrain_should_place_feature(seed_value, world_block_x,
                    world_block_z, TERRAIN_FEATURE_SHRUB_SALT,
                    config.biomes[biome].shrub_chance_percent) == FT_TRUE)
            {
                error_code = chunk.write_block(local_x,
                    column_height + TERRAIN_FEATURE_SHRUB_HEIGHT_OFFSET,
                    local_z, TERRAIN_GENERATOR_SHRUB_BLOCK);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
            }
            if (column_height < config.sea_level
                && terrain_should_place_feature(seed_value, world_block_x,
                    world_block_z, TERRAIN_FEATURE_WATER_SALT,
                    config.water_chance_percent) == FT_TRUE)
            {
                local_y = column_height + 1;
                while (local_y <= config.sea_level
                    && local_y < GAME_VOXEL_CHUNK_HEIGHT)
                {
                    error_code = chunk.write_block(local_x, local_y, local_z,
                        TERRAIN_GENERATOR_WATER_BLOCK);
                    if (error_code != FT_ERR_SUCCESS)
                        return (error_code);
                    local_y += 1;
                }
            }
            local_x += 1;
        }
        local_z += 1;
    }
    local_z = 2;
    while (local_z + 2 < GAME_VOXEL_CHUNK_DEPTH)
    {
        world_block_z = world_block_origin_z + local_z;
        local_x = 2;
        while (local_x + 2 < GAME_VOXEL_CHUNK_WIDTH)
        {
            column_index = (local_z * GAME_VOXEL_CHUNK_WIDTH) + local_x;
            world_block_x = world_block_origin_x + local_x;
            biome = column_cache[column_index].biome;
            if (column_cache[column_index].can_place_trees == FT_TRUE)
            {
                tree_feature_seed = terrain_feature_seed(seed_value,
                    world_block_x, world_block_z, TERRAIN_FEATURE_TREE_SALT);
                if ((tree_feature_seed % 100U)
                    < config.biomes[biome].tree_chance_percent)
                {
                    tree_template = config.biomes[biome].tree_template;
                    if (tree_template == ft_nullptr)
                        tree_template = &terrain_tree_template_for_biome(
                            static_cast<terrain_biome>(biome), tree_feature_seed);
                    column_height = column_cache[column_index].column_height;
                    if (terrain_can_place_tree_template(chunk, local_x,
                            column_height + 1, local_z, *tree_template)
                        == FT_TRUE)
                    {
                        error_code = terrain_place_tree_template(chunk,
                            local_x, column_height + 1, local_z,
                            *tree_template);
                        if (error_code != FT_ERR_SUCCESS)
                            return (error_code);
                    }
                }
            }
            local_x += 4;
        }
        local_z += 4;
    }
    feature_index = 0U;
    while (feature_index < config.feature_count
        && feature_index < TERRAIN_MAX_FEATURE_RULES)
    {
        feature_rule = &config.features[feature_index];
        if (feature_rule->template_data != ft_nullptr)
        {
            local_z = 2;
            while (local_z + 2 < GAME_VOXEL_CHUNK_DEPTH)
            {
                local_x = 2;
                while (local_x + 2 < GAME_VOXEL_CHUNK_WIDTH)
                {
                    column_index = (local_z * GAME_VOXEL_CHUNK_WIDTH) + local_x;
                    biome = column_cache[column_index].biome;
                    column_height = column_cache[column_index].column_height;
                    if ((feature_rule->biome_index < 0
                            || feature_rule->biome_index
                                == static_cast<int32_t>(biome))
                        && column_height >= feature_rule->minimum_height
                        && column_height <= feature_rule->maximum_height
                        && (feature_rule->requires_dry_land == FT_FALSE
                            || column_height >= config.sea_level))
                    {
                        feature_seed = terrain_feature_seed(seed_value,
                            world_block_origin_x + local_x,
                            world_block_origin_z + local_z,
                            TERRAIN_FEATURE_TREE_SALT
                                ^ static_cast<uint64_t>(feature_index + 1U));
                        if ((feature_seed % 100U)
                            < feature_rule->chance_percent
                            && terrain_can_place_tree_template(chunk, local_x,
                                column_height + 1, local_z,
                                *feature_rule->template_data) == FT_TRUE)
                        {
                            error_code = terrain_place_tree_template(chunk,
                                local_x, column_height + 1, local_z,
                                *feature_rule->template_data);
                            if (error_code != FT_ERR_SUCCESS)
                                return (error_code);
                        }
                    }
                    local_x += 4;
                }
                local_z += 4;
            }
        }
        feature_index += 1U;
    }
    chunk.clear_dirty();
    return (FT_ERR_SUCCESS);
}

#endif
