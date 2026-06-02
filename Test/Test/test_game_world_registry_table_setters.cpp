#include "../test_internal.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/map.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_registry_setters)
{
    game_world_registry registry;
    ft_map<int, game_region_definition> regions;
    ft_map<int, game_world_region> worlds;
    ft_vector<int> region_ids;
    game_region_definition fetched_region;
    game_world_region fetched_world;
    game_region_definition region;
    game_world_region world;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, worlds.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string cave_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cave_name.initialize("cave"));
    region.set_name(cave_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string dark_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dark_description.initialize("dark"));
    region.set_description(dark_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.push_back(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(5, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());
    regions.insert(2, region);
    worlds.insert(5, world);

    registry.set_regions(regions);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    registry.set_world_regions(worlds);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    regions.clear();
    worlds.clear();

    FT_ASSERT_EQ(1, registry.get_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(1, registry.get_world_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_region(2, fetched_region));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_STR_EQ("cave", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.fetch_world(5, fetched_world));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(2, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_world.get_error());

    registry.get_regions().clear();
    registry.get_world_regions().clear();
    FT_ASSERT_EQ(0, registry.get_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(0, registry.get_world_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    registry.set_regions(regions);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    registry.set_world_regions(worlds);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(0, registry.get_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    FT_ASSERT_EQ(0, registry.get_world_regions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registry.get_error());
    return (1);
}
