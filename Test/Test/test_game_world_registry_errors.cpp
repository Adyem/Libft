#include "../../Game/game_world_registry.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_world_registry_fetch_region_sets_errno, "fetching missing region updates errno and registry error")
{
    ft_world_registry registry;
    ft_region_definition region;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_region(77, region));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    return (1);
}

FT_TEST(test_world_registry_fetch_world_sets_errno, "fetching missing world updates errno and registry error")
{
    ft_world_registry registry;
    ft_world_region world;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.fetch_world(33, world));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, registry.get_error());
    return (1);
}

FT_TEST(test_world_registry_get_error_str_reports_latest, "get_error_str matches last operation error")
{
    ft_world_registry registry;
    ft_region_definition region;

    registry.fetch_region(9, region);
    FT_ASSERT_EQ(ft_strerror(FT_ERR_NOT_FOUND), registry.get_error_str());
    return (1);
}

FT_TEST(test_world_registry_register_region_restores_errno, "successful register_region leaves incoming errno intact")
{
    ft_world_registry registry;
    ft_region_definition region(6, ft_string("lake"), ft_string("blue water"), 4);

    ft_errno = FT_ERR_GAME_GENERAL_ERROR;
    FT_ASSERT_EQ(ER_SUCCESS, registry.register_region(region));
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, registry.get_error());
    return (1);
}

FT_TEST(test_world_registry_set_world_regions_restores_errno, "set_world_regions maintains errno while replacing data")
{
    ft_world_registry registry;
    ft_map<int, ft_world_region> world_regions;
    ft_vector<int> region_ids;
    ft_world_region fetched_world;

    region_ids.push_back(12);
    world_regions.insert(3, ft_world_region(3, region_ids));
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    registry.set_world_regions(world_regions);

    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, registry.fetch_world(3, fetched_world));
    FT_ASSERT_EQ(12, fetched_world.get_region_ids()[0]);
    return (1);
}
