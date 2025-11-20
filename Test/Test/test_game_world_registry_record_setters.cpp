#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_region_definition_setters, "region definition setters update fields")
{
    ft_region_definition region;

    region = ft_region_definition(1, ft_string("old"), ft_string("desc"), 2);
    region.set_region_id(5);
    region.set_name(ft_string("new"));
    region.set_description(ft_string("details"));
    region.set_recommended_level(9);

    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(ft_string("new"), region.get_name());
    FT_ASSERT_EQ(ft_string("details"), region.get_description());
    FT_ASSERT_EQ(9, region.get_recommended_level());
    return (1);
}

FT_TEST(test_world_region_setters, "world region setters replace world and regions")
{
    ft_world_region world;
    ft_vector<int> region_ids;

    region_ids.push_back(6);
    region_ids.push_back(7);
    world = ft_world_region(3, region_ids);

    world.set_world_id(10);
    region_ids.clear();
    region_ids.push_back(20);
    world.set_region_ids(region_ids);

    FT_ASSERT_EQ(10, world.get_world_id());
    FT_ASSERT_EQ(1, world.get_region_ids().size());
    FT_ASSERT_EQ(20, world.get_region_ids()[0]);
    return (1);
}
