#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "../../Modules/Voxel/voxel.hpp"

static int32_t test_terrain_surface_height(game_voxel_chunk &chunk,
    int32_t local_x, int32_t local_z) noexcept
{
    int32_t local_y;
    uint32_t block_id;
    int32_t error_code;

    local_y = GAME_VOXEL_CHUNK_HEIGHT - 1;
    while (local_y >= 0)
    {
        error_code = chunk.read_block(local_x, local_y, local_z, &block_id);
        if (error_code != FT_ERR_SUCCESS)
            return (-1);
        if (block_id != GAME_VOXEL_AIR_BLOCK)
            return (local_y);
        local_y -= 1;
    }
    return (-1);
}

FT_TEST(test_terrain_generate_chunk_generates_default_surface_chunk)
{
    game_voxel_chunk chunk;
    terrain_biome biome;
    terrain_biome_profile biome_profile;
    int32_t surface_height;
    ft_bool surface_in_range;
    int32_t surface_variation_limit;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk,
        "terrain-test-seed"));
    biome = terrain_get_biome(0, 0, "terrain-test-seed");
    biome_profile = terrain_get_biome_profile(biome);
    surface_height = test_terrain_surface_height(chunk, 0, 0);
    FT_ASSERT_NEQ(-1, surface_height);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, surface_height, 0,
        &block_id));
    FT_ASSERT_EQ(terrain_surface_block_for_biome(biome), block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, surface_height + 1, 0,
        &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    surface_in_range = FT_TRUE;
    surface_variation_limit = biome_profile.height_variation
        + (biome_profile.height_variation / 2);
    if (surface_height < biome_profile.surface_height
        - surface_variation_limit)
        surface_in_range = FT_FALSE;
    if (surface_height > biome_profile.surface_height
        + surface_variation_limit)
        surface_in_range = FT_FALSE;
    FT_ASSERT_EQ(FT_TRUE, surface_in_range);
    FT_ASSERT_EQ(FT_FALSE, chunk.is_dirty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_get_biome_changes_across_zone_boundaries)
{
    terrain_biome left_biome;
    terrain_biome right_biome;

    left_biome = terrain_get_biome(0, 0, "terrain-test-seed");
    right_biome = terrain_get_biome(TERRAIN_BIOME_ZONE_WIDTH, 0,
        "terrain-test-seed");
    FT_ASSERT_NEQ(left_biome, right_biome);
    return (1);
}

FT_TEST(test_terrain_surface_helpers_match_biome_rules)
{
    FT_ASSERT_EQ(TERRAIN_GENERATOR_GRASS_BLOCK,
        terrain_surface_block_for_biome(TERRAIN_BIOME_PLAINS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_MOSS_ROCK_BLOCK,
        terrain_surface_block_for_biome(TERRAIN_BIOME_HILLS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_SAND_BLOCK,
        terrain_surface_block_for_biome(TERRAIN_BIOME_DESERT));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_SNOW_BLOCK,
        terrain_surface_block_for_biome(TERRAIN_BIOME_SNOW));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_SLATE_BLOCK,
        terrain_surface_block_for_biome(TERRAIN_BIOME_MOUNTAINS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_DIRT_BLOCK,
        terrain_subsurface_block_for_biome(TERRAIN_BIOME_PLAINS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_MOSS_ROCK_BLOCK,
        terrain_subsurface_block_for_biome(TERRAIN_BIOME_HILLS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CANYON_ROCK_BLOCK,
        terrain_subsurface_block_for_biome(TERRAIN_BIOME_DESERT));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_PERMAFROST_BLOCK,
        terrain_subsurface_block_for_biome(TERRAIN_BIOME_SNOW));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_STONE_BLOCK,
        terrain_subsurface_block_for_biome(TERRAIN_BIOME_MOUNTAINS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_STONE_BLOCK,
        terrain_deep_block_for_biome(TERRAIN_BIOME_PLAINS));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CANYON_ROCK_BLOCK,
        terrain_deep_block_for_biome(TERRAIN_BIOME_MOUNTAINS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_shrubs(TERRAIN_BIOME_PLAINS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_shrubs(TERRAIN_BIOME_HILLS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_shrubs(TERRAIN_BIOME_DESERT));
    FT_ASSERT_EQ(FT_FALSE, terrain_biome_has_shrubs(TERRAIN_BIOME_SNOW));
    FT_ASSERT_EQ(FT_FALSE, terrain_biome_has_shrubs(TERRAIN_BIOME_MOUNTAINS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_trees(TERRAIN_BIOME_PLAINS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_trees(TERRAIN_BIOME_HILLS));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_trees(TERRAIN_BIOME_DESERT));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_trees(TERRAIN_BIOME_SNOW));
    FT_ASSERT_EQ(FT_TRUE, terrain_biome_has_trees(TERRAIN_BIOME_MOUNTAINS));
    return (1);
}

FT_TEST(test_terrain_tree_templates_expose_expected_blocks)
{
    const terrain_tree_template &oak_tree_template =
        terrain_small_oak_tree_template();
    const terrain_tree_template &oak_tree_template_variant =
        terrain_small_oak_tree_template_variant(1U);
    const terrain_tree_template &pine_tree_template =
        terrain_small_pine_tree_template();
    const terrain_tree_template &pine_tree_template_variant =
        terrain_small_pine_tree_template_variant(2U);
    const terrain_tree_template &cactus_tree_template =
        terrain_small_cactus_tree_template();
    const terrain_tree_template &cactus_tree_template_variant =
        terrain_small_cactus_tree_template_variant(1U);
    const terrain_tree_template &large_oak_tree_template =
        terrain_large_oak_tree_template();
    const terrain_tree_template &large_oak_tree_template_variant =
        terrain_large_oak_tree_template_variant(1U);
    const terrain_tree_template &large_pine_tree_template =
        terrain_large_pine_tree_template();
    const terrain_tree_template &large_pine_tree_template_variant =
        terrain_large_pine_tree_template_variant(1U);

    FT_ASSERT_NEQ(0U, oak_tree_template.block_count);
    FT_ASSERT_NEQ(oak_tree_template.block_count,
        oak_tree_template_variant.block_count);
    FT_ASSERT_NEQ(0U, pine_tree_template.block_count);
    FT_ASSERT_NEQ(pine_tree_template.block_count,
        pine_tree_template_variant.block_count);
    FT_ASSERT_NEQ(0U, cactus_tree_template.block_count);
    FT_ASSERT_NEQ(cactus_tree_template.block_count,
        cactus_tree_template_variant.block_count);
    FT_ASSERT_NEQ(oak_tree_template.block_count,
        large_oak_tree_template.block_count);
    FT_ASSERT_NEQ(pine_tree_template.block_count,
        large_pine_tree_template.block_count);
    FT_ASSERT_NEQ(large_oak_tree_template.block_count,
        large_oak_tree_template_variant.block_count);
    FT_ASSERT_NEQ(large_pine_tree_template.block_count,
        large_pine_tree_template_variant.block_count);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        oak_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        pine_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CACTUS_BLOCK,
        cactus_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        oak_tree_template_variant.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        pine_tree_template_variant.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CACTUS_BLOCK,
        cactus_tree_template_variant.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        large_oak_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        large_pine_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        large_oak_tree_template_variant.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        large_pine_tree_template_variant.blocks[0].block_id);
    return (1);
}

FT_TEST(test_terrain_place_tree_template_writes_small_oak_tree)
{
    game_voxel_chunk chunk;
    const terrain_tree_template &oak_tree_template =
        terrain_small_oak_tree_template();
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_place_tree_template(chunk, 8, 12, 8,
        oak_tree_template));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 12, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 16, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LEAVES_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_place_tree_template_writes_small_pine_tree)
{
    game_voxel_chunk chunk;
    const terrain_tree_template &pine_tree_template =
        terrain_small_pine_tree_template();
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_place_tree_template(chunk, 8, 12, 8,
        pine_tree_template));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 12, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 18, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LEAVES_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_place_tree_template_writes_small_cactus)
{
    game_voxel_chunk chunk;
    const terrain_tree_template &cactus_tree_template =
        terrain_small_cactus_tree_template();
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_place_tree_template(chunk, 8, 12, 8,
        cactus_tree_template));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 12, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CACTUS_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(8, 15, 8, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CACTUS_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_can_place_tree_template_rejects_occupied_space)
{
    game_voxel_chunk chunk;
    const terrain_tree_template &oak_tree_template =
        terrain_small_oak_tree_template();

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(8, 12, 8,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_FALSE, terrain_can_place_tree_template(chunk, 8, 12, 8,
        oak_tree_template));
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, terrain_place_tree_template(chunk,
        8, 12, 8, oak_tree_template));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_tree_template_for_biome_uses_seed_selected_large_variants)
{
    const terrain_tree_template &oak_tree_template =
        terrain_tree_template_for_biome(TERRAIN_BIOME_HILLS, 3U);
    const terrain_tree_template &pine_tree_template =
        terrain_tree_template_for_biome(TERRAIN_BIOME_SNOW, 4U);

    FT_ASSERT_EQ(terrain_large_oak_tree_template().block_count,
        oak_tree_template.block_count);
    FT_ASSERT_EQ(terrain_large_pine_tree_template(1U).block_count,
        pine_tree_template.block_count);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        oak_tree_template.blocks[0].block_id);
    FT_ASSERT_EQ(TERRAIN_GENERATOR_OAK_LOG_BLOCK,
        pine_tree_template.blocks[0].block_id);
    return (1);
}

FT_TEST(test_terrain_generate_chunk_uses_biome_profile)
{
    game_voxel_chunk left_chunk;
    game_voxel_chunk right_chunk;
    terrain_biome left_biome;
    terrain_biome right_biome;
    terrain_biome_profile left_profile;
    terrain_biome_profile right_profile;
    int32_t left_surface_height;
    int32_t right_surface_height;
    ft_bool left_height_in_range;
    ft_bool right_height_in_range;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(left_chunk, 0, 0,
        "terrain-test-seed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(right_chunk,
        TERRAIN_BIOME_ZONE_WIDTH, 0, "terrain-test-seed"));
    left_biome = terrain_get_biome(0, 0, "terrain-test-seed");
    right_biome = terrain_get_biome(TERRAIN_BIOME_ZONE_WIDTH, 0,
        "terrain-test-seed");
    FT_ASSERT_NEQ(left_biome, right_biome);
    left_profile = terrain_get_biome_profile(left_biome);
    right_profile = terrain_get_biome_profile(right_biome);
    left_surface_height = test_terrain_surface_height(left_chunk, 0, 0);
    right_surface_height = test_terrain_surface_height(right_chunk, 0, 0);
    FT_ASSERT_NEQ(-1, left_surface_height);
    FT_ASSERT_NEQ(-1, right_surface_height);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_chunk.read_block(0, left_surface_height,
        0, &block_id));
    FT_ASSERT_EQ(terrain_surface_block_for_biome(left_biome), block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_chunk.read_block(0,
        right_surface_height, 0, &block_id));
    FT_ASSERT_EQ(terrain_surface_block_for_biome(right_biome), block_id);
    left_height_in_range = FT_TRUE;
    if (left_surface_height < left_profile.surface_height
        - left_profile.height_variation
        - (left_profile.height_variation / 2))
        left_height_in_range = FT_FALSE;
    if (left_surface_height > left_profile.surface_height
        + left_profile.height_variation
        + (left_profile.height_variation / 2))
        left_height_in_range = FT_FALSE;
    right_height_in_range = FT_TRUE;
    if (right_surface_height < right_profile.surface_height
        - right_profile.height_variation
        - (right_profile.height_variation / 2))
        right_height_in_range = FT_FALSE;
    if (right_surface_height > right_profile.surface_height
        + right_profile.height_variation
        + (right_profile.height_variation / 2))
        right_height_in_range = FT_FALSE;
    FT_ASSERT_EQ(FT_TRUE, left_height_in_range);
    FT_ASSERT_EQ(FT_TRUE, right_height_in_range);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generate_chunk_height_varies_with_world_position)
{
    game_voxel_chunk left_chunk;
    game_voxel_chunk right_chunk;
    int32_t left_surface_height;
    int32_t right_surface_height;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(left_chunk, 0, 0,
        "terrain-test-seed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(right_chunk, 32, 0,
        "terrain-test-seed"));
    left_surface_height = test_terrain_surface_height(left_chunk, 0, 0);
    right_surface_height = test_terrain_surface_height(right_chunk, 0, 0);
    FT_ASSERT_NEQ(-1, left_surface_height);
    FT_ASSERT_NEQ(-1, right_surface_height);
    FT_ASSERT_NEQ(left_surface_height, right_surface_height);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generate_chunk_is_deterministic_for_same_seed_and_origin)
{
    game_voxel_chunk first_chunk;
    game_voxel_chunk second_chunk;
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t first_block_id;
    uint32_t second_block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(first_chunk, 128, 256,
        "terrain-test-seed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(second_chunk, 128, 256,
        "terrain-test-seed"));
    local_z = 0;
    while (local_z < GAME_VOXEL_CHUNK_DEPTH)
    {
        local_y = 0;
        while (local_y < GAME_VOXEL_CHUNK_HEIGHT)
        {
            local_x = 0;
            while (local_x < GAME_VOXEL_CHUNK_WIDTH)
            {
                FT_ASSERT_EQ(FT_ERR_SUCCESS, first_chunk.read_block(local_x,
                    local_y, local_z, &first_block_id));
                FT_ASSERT_EQ(FT_ERR_SUCCESS, second_chunk.read_block(local_x,
                    local_y, local_z, &second_block_id));
                FT_ASSERT_EQ(first_block_id, second_block_id);
                local_x += 1;
            }
            local_y += 1;
        }
        local_z += 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_chunk.destroy());
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

static uint32_t test_custom_biome_selector(uint64_t, int32_t, int32_t,
    uint32_t biome_count, void *) noexcept
{
    return (biome_count - 1U);
}

FT_TEST(test_terrain_generation_config_controls_custom_flat_biome_and_water)
{
    game_voxel_chunk chunk;
    terrain_generation_config config = terrain_default_generation_config();
    uint32_t block_id;

    config.biome_count = 1U;
    config.large_noise_scale = 1;
    config.detail_noise_scale = 1;
    config.detail_noise_percent = 0;
    config.sea_level = 20;
    config.water_chance_percent = 0U;
    config.biomes[0].profile.surface_height = 40;
    config.biomes[0].profile.height_variation = 0;
    config.biomes[0].profile.topsoil_depth = 0;
    config.biomes[0].surface_block_id = TERRAIN_GENERATOR_SAND_BLOCK;
    config.biomes[0].subsurface_block_id = TERRAIN_GENERATOR_SAND_BLOCK;
    config.biomes[0].deep_block_id = TERRAIN_GENERATOR_STONE_BLOCK;
    config.biomes[0].allow_shrubs = FT_FALSE;
    config.biomes[0].allow_trees = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk, 0, 0,
        "custom-config", config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, 40, 0, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_SAND_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, 41, 0, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_config_accepts_custom_feature_rule)
{
    game_voxel_chunk chunk;
    terrain_generation_config config = terrain_default_generation_config();
    const terrain_tree_template &cactus = terrain_small_cactus_tree_template();
    uint32_t block_id;
    int32_t cactus_count = 0;
    int32_t x;
    int32_t y;
    int32_t z;

    config.feature_count = 1U;
    config.features[0].template_data = &cactus;
    config.features[0].biome_index = -1;
    config.features[0].chance_percent = 100U;
    config.features[0].minimum_height = 0;
    config.features[0].maximum_height = GAME_VOXEL_CHUNK_HEIGHT;
    config.features[0].requires_dry_land = FT_TRUE;
    config.biomes[0].allow_trees = FT_FALSE;
    config.biomes[0].allow_shrubs = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk, 0, 0,
        "feature-config", config));
    z = 0;
    while (z < GAME_VOXEL_CHUNK_DEPTH)
    {
        y = 0;
        while (y < GAME_VOXEL_CHUNK_HEIGHT)
        {
            x = 0;
            while (x < GAME_VOXEL_CHUNK_WIDTH)
            {
                FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(x, y, z, &block_id));
                if (block_id == TERRAIN_GENERATOR_CACTUS_BLOCK)
                    cactus_count += 1;
                x += 1;
            }
            y += 1;
        }
        z += 1;
    }
    FT_ASSERT_NEQ(0, cactus_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_config_can_select_custom_biome_slot)
{
    game_voxel_chunk chunk;
    terrain_generation_config config = terrain_default_generation_config();
    uint32_t block_id;

    config.biome_count = 6U;
    config.biome_selector = &test_custom_biome_selector;
    config.biomes[5].profile.surface_height = 50;
    config.biomes[5].profile.height_variation = 0;
    config.biomes[5].profile.topsoil_depth = 0;
    config.biomes[5].surface_block_id = TERRAIN_GENERATOR_CANYON_ROCK_BLOCK;
    config.biomes[5].subsurface_block_id = TERRAIN_GENERATOR_CANYON_ROCK_BLOCK;
    config.biomes[5].deep_block_id = TERRAIN_GENERATOR_SLATE_BLOCK;
    config.biomes[5].allow_shrubs = FT_FALSE;
    config.biomes[5].allow_trees = FT_FALSE;
    config.sea_level = 0;
    config.water_chance_percent = 0U;
    FT_ASSERT_EQ(5U, terrain_get_biome_index(config, 0, 0, "custom-biome"));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk, 0, 0,
        "custom-biome", config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, 50, 0, &block_id));
    FT_ASSERT_EQ(TERRAIN_GENERATOR_CANYON_ROCK_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_rejects_custom_tree_without_template)
{
    terrain_generation_config config = terrain_default_generation_config();

    config.biome_count = 6U;
    config.biomes[5].allow_trees = FT_TRUE;
    config.biomes[5].tree_template = ft_nullptr;
    FT_ASSERT_EQ(FT_FALSE, terrain_generation_config_is_valid(config));
    return (1);
}

FT_TEST(test_terrain_generate_chunk_clears_previous_voxel_data)
{
    game_voxel_chunk regenerated_chunk;
    game_voxel_chunk fresh_chunk;
    terrain_generation_config high_config =
        terrain_default_generation_config();
    terrain_generation_config low_config = high_config;
    uint32_t regenerated_block_id;
    uint32_t fresh_block_id;
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;

    high_config.biome_count = 1U;
    high_config.sea_level = 0;
    high_config.water_chance_percent = 0U;
    high_config.biomes[0].profile.surface_height = 120;
    high_config.biomes[0].profile.height_variation = 0;
    high_config.biomes[0].profile.topsoil_depth = 0;
    high_config.biomes[0].allow_shrubs = FT_FALSE;
    high_config.biomes[0].allow_trees = FT_FALSE;
    low_config.biomes[0].profile.surface_height = 20;
    low_config.biomes[0].profile.height_variation = 0;
    low_config.biomes[0].profile.topsoil_depth = 0;
    low_config.biomes[0].allow_shrubs = FT_FALSE;
    low_config.biomes[0].allow_trees = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, regenerated_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fresh_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(regenerated_chunk,
        0, 0, "regeneration-test", high_config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(regenerated_chunk,
        0, 0, "regeneration-test", low_config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(fresh_chunk,
        0, 0, "regeneration-test", low_config));
    local_z = 0;
    while (local_z < GAME_VOXEL_CHUNK_DEPTH)
    {
        local_y = 0;
        while (local_y < GAME_VOXEL_CHUNK_HEIGHT)
        {
            local_x = 0;
            while (local_x < GAME_VOXEL_CHUNK_WIDTH)
            {
                FT_ASSERT_EQ(FT_ERR_SUCCESS, regenerated_chunk.read_block(
                    local_x, local_y, local_z, &regenerated_block_id));
                FT_ASSERT_EQ(FT_ERR_SUCCESS, fresh_chunk.read_block(local_x,
                    local_y, local_z, &fresh_block_id));
                FT_ASSERT_EQ(fresh_block_id, regenerated_block_id);
                local_x += 1;
            }
            local_y += 1;
        }
        local_z += 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fresh_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regenerated_chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_config_can_make_plains_uneven)
{
    game_voxel_chunk chunk;
    terrain_generation_config config = terrain_default_generation_config();
    int32_t x;
    int32_t z;
    int32_t surface_height;
    int32_t minimum_height = GAME_VOXEL_CHUNK_HEIGHT;
    int32_t maximum_height = 0;

    config.biome_count = 1U;
    config.sea_level = 0;
    config.water_chance_percent = 0U;
    config.biomes[0].profile.surface_height = 40;
    config.biomes[0].profile.height_variation = 12;
    config.biomes[0].profile.topsoil_depth = 0;
    config.biomes[0].allow_shrubs = FT_FALSE;
    config.biomes[0].allow_trees = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk, 0, 0,
        "uneven-plains", config));
    z = 0;
    while (z < GAME_VOXEL_CHUNK_DEPTH)
    {
        x = 0;
        while (x < GAME_VOXEL_CHUNK_WIDTH)
        {
            surface_height = test_terrain_surface_height(chunk, x, z);
            FT_ASSERT_NEQ(-1, surface_height);
            if (surface_height < minimum_height)
                minimum_height = surface_height;
            if (surface_height > maximum_height)
                maximum_height = surface_height;
            x += 1;
        }
        z += 1;
    }
    FT_ASSERT_NEQ(minimum_height, maximum_height);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_config_rejects_invalid_ranges)
{
    game_voxel_chunk chunk;
    terrain_generation_config config = terrain_default_generation_config();

    config.biome_count = 0U;
    FT_ASSERT_EQ(FT_FALSE, terrain_generation_config_is_valid(config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, terrain_generate_chunk(chunk, 0, 0,
        "invalid-config", config));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_terrain_generation_config_controls_tree_and_water_density)
{
    game_voxel_chunk dry_chunk;
    game_voxel_chunk wet_chunk;
    terrain_generation_config config = terrain_default_generation_config();
    const terrain_tree_template &oak = terrain_small_oak_tree_template();
    uint32_t block_id;
    int32_t tree_count = 0;
    int32_t dense_tree_count = 0;
    int32_t water_count = 0;
    int32_t x;
    int32_t y;
    int32_t z;

    config.biome_count = 1U;
    config.sea_level = 50;
    config.water_chance_percent = 0U;
    config.biomes[0].profile.surface_height = 40;
    config.biomes[0].profile.height_variation = 0;
    config.biomes[0].profile.topsoil_depth = 0;
    config.biomes[0].allow_shrubs = FT_FALSE;
    config.biomes[0].allow_trees = FT_TRUE;
    config.biomes[0].tree_chance_percent = 0U;
    config.biomes[0].tree_template = &oak;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dry_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(dry_chunk, 0, 0,
        "density-config", config));
    config.water_chance_percent = 100U;
    config.biomes[0].tree_chance_percent = 100U;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, wet_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(wet_chunk, 0, 0,
        "density-config", config));
    z = 0;
    while (z < GAME_VOXEL_CHUNK_DEPTH)
    {
        y = 0;
        while (y < GAME_VOXEL_CHUNK_HEIGHT)
        {
            x = 0;
            while (x < GAME_VOXEL_CHUNK_WIDTH)
            {
                FT_ASSERT_EQ(FT_ERR_SUCCESS, dry_chunk.read_block(x, y, z,
                    &block_id));
                if (block_id == TERRAIN_GENERATOR_OAK_LOG_BLOCK)
                    tree_count += 1;
                FT_ASSERT_EQ(FT_ERR_SUCCESS, wet_chunk.read_block(x, y, z,
                    &block_id));
                if (block_id == TERRAIN_GENERATOR_OAK_LOG_BLOCK)
                    dense_tree_count += 1;
                if (block_id == TERRAIN_GENERATOR_WATER_BLOCK)
                    water_count += 1;
                x += 1;
            }
            y += 1;
        }
        z += 1;
    }
    FT_ASSERT_EQ(0, tree_count);
    FT_ASSERT_NEQ(0, dense_tree_count);
    FT_ASSERT_NEQ(0, water_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, wet_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dry_chunk.destroy());
    return (1);
}

#endif
