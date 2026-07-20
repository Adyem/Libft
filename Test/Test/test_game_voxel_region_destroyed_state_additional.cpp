#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void region_destroyed_set_storage_path(game_voxel_region &value)
{
    (void)value.set_region_storage_path("region");
    return ;
}

static void region_destroyed_load_region(game_voxel_region &value)
{
    (void)value.load_region(0, 0);
    return ;
}

static void region_destroyed_save_region(game_voxel_region &value)
{
    (void)value.save_region();
    return ;
}

static void region_destroyed_load_chunk(game_voxel_region &value)
{
    game_voxel_chunk *chunk_pointer;

    chunk_pointer = ft_nullptr;
    (void)value.load_chunk(0, 0, &chunk_pointer);
    return ;
}

static void region_destroyed_read_block(game_voxel_region &value)
{
    uint32_t block_id;

    block_id = 0;
    (void)value.read_block(0, 0, 0, &block_id);
    return ;
}

static void region_destroyed_write_block(game_voxel_region &value)
{
    (void)value.write_block(0, 0, 0, 1);
    return ;
}

static void region_destroyed_get_region_start_x(game_voxel_region &value)
{
    (void)value.get_region_start_x();
    return ;
}

static void region_destroyed_get_region_start_z(game_voxel_region &value)
{
    (void)value.get_region_start_z();
    return ;
}

FT_TEST(test_game_voxel_region_destroyed_set_storage_path_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_set_storage_path));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_load_region_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_load_region));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_save_region_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_save_region));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_load_chunk_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_load_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_read_block_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_read_block));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_write_block_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_write_block));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_get_region_start_x_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_get_region_start_x));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_get_region_start_z_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_get_region_start_z));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_get_error_is_valid)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_destructor_is_non_aborting)
{
    game_voxel_region *value;

    value = new game_voxel_region();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
