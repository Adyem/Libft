#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/Geometry/geometry_3d.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void region_destroyed_save_chunk(game_voxel_region &value)
{
    game_voxel_chunk chunk;

    (void)chunk.initialize();
    (void)value.save_chunk(0, 0, chunk);
    return ;
}

FT_TEST(test_game_voxel_region_destroyed_save_chunk_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_voxel_region>(
                        region_destroyed_save_chunk));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_has_chunk_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.has_chunk(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_is_chunk_loaded_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_chunk_loaded(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_is_chunk_visible_is_safe)
{
    game_voxel_region value;
    geometry_frustum frustum;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_chunk_visible(0, 0, frustum));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_get_error_str_is_valid)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_state_can_reinitialize_again)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.has_chunk(0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_cleanup_is_idempotent_again)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_destructor_after_cleanup_is_safe_again)
{
    game_voxel_region *value;

    value = new game_voxel_region();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_reinitialized_write_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.write_block(0, 0, 0, 0));
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_error_remains_success)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_voxel_region_destroyed_cleanup_can_repeat)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
