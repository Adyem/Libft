#include "../test_internal.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_region_definition_setters)
{
    game_region_definition region;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string old_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, old_name.initialize("old"));
    region.set_name(old_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string old_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, old_description.initialize("desc"));
    region.set_description(old_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string new_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_name.initialize("new"));
    region.set_name(new_name);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string new_description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, new_description.initialize("details"));
    region.set_description(new_description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());

    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("new", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("details", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(9, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_world_region_setters)
{
    ft_vector<int> region_ids;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.push_back(6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.push_back(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    game_world_region world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    world.set_world_id(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    region_ids.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.push_back(20));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.get_error());
    world.set_region_ids(region_ids);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    FT_ASSERT_EQ(10, world.get_world_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(1, world.get_region_ids().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(20, world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}
