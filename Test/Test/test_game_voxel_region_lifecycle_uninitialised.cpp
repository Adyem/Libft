#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_voxel_region_set_region_storage_path(game_voxel_region &value)
{
    value.set_region_storage_path("region");
    return ;
}

static void game_voxel_region_load_region(game_voxel_region &value)
{
    (void)value.load_region(0, 0);
    return ;
}

static void game_voxel_region_save_region(game_voxel_region &value)
{
    (void)value.save_region();
    return ;
}

static void game_voxel_region_load_chunk(game_voxel_region &value)
{
    game_voxel_chunk *chunk = ft_nullptr;

    (void)value.load_chunk(0, 0, &chunk);
    return ;
}

static void game_voxel_region_save_chunk(game_voxel_region &value)
{
    game_voxel_chunk chunk;

    (void)value.save_chunk(0, 0, chunk);
    return ;
}

static void game_voxel_region_read_block(game_voxel_region &value)
{
    uint32_t block_id = 0U;

    (void)value.read_block(0, 0, 0, &block_id);
    return ;
}

static void game_voxel_region_write_block(game_voxel_region &value)
{
    (void)value.write_block(0, 0, 0, 1U);
    return ;
}

static void game_voxel_region_has_chunk(game_voxel_region &value)
{
    (void)value.has_chunk(0, 0);
    return ;
}

static void game_voxel_region_is_chunk_loaded(game_voxel_region &value)
{
    (void)value.is_chunk_loaded(0, 0);
    return ;
}

static void game_voxel_region_is_chunk_visible(game_voxel_region &value)
{
    geometry_frustum frustum;

    (void)value.is_chunk_visible(0, 0, frustum);
    return ;
}

static void game_voxel_region_get_region_start_x(game_voxel_region &value)
{
    (void)value.get_region_start_x();
    return ;
}

static void game_voxel_region_get_region_start_z(game_voxel_region &value)
{
    (void)value.get_region_start_z();
    return ;
}

static void game_voxel_region_get_error(game_voxel_region &value)
{
    (void)value.get_error();
    return ;
}

static void game_voxel_region_get_error_str(game_voxel_region &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_voxel_region_set_region_storage_path_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_set_region_storage_path));
    return (1);
}

FT_TEST(test_game_voxel_region_load_region_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_load_region));
    return (1);
}

FT_TEST(test_game_voxel_region_save_region_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_save_region));
    return (1);
}

FT_TEST(test_game_voxel_region_load_chunk_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_load_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_save_chunk_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_save_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_read_block_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_read_block));
    return (1);
}

FT_TEST(test_game_voxel_region_write_block_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_write_block));
    return (1);
}

FT_TEST(test_game_voxel_region_has_chunk_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_has_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_is_chunk_loaded_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_is_chunk_loaded));
    return (1);
}

FT_TEST(test_game_voxel_region_is_chunk_visible_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_is_chunk_visible));
    return (1);
}

FT_TEST(test_game_voxel_region_get_region_start_x_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_get_region_start_x));
    return (1);
}

FT_TEST(test_game_voxel_region_get_region_start_z_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_get_region_start_z));
    return (1);
}

FT_TEST(test_game_voxel_region_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_get_error));
    return (1);
}

FT_TEST(test_game_voxel_region_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        game_voxel_region_get_error_str));
    return (1);
}
