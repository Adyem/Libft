#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void voxel_region_initialize_twice(game_voxel_region &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_voxel_region_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_voxel_region>(
                        voxel_region_initialize_twice));
    return (1);
}

FT_TEST(test_game_voxel_region_initialize_succeeds_clean)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_voxel_region_destroy_twice_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_reinitialize_after_destroy)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_voxel_region_self_move_is_safe)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}

FT_TEST(test_game_voxel_region_error_after_success)
{
    game_voxel_region value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}
