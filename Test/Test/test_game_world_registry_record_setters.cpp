#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_region_definition_setters, "region definition setters update fields")
{
    ft_region_definition region;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_name(ft_string("old"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_description(ft_string("desc"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_name(ft_string("new"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_description(ft_string("details"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());

    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(ft_string("new"), region.get_name());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(ft_string("details"), region.get_description());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(9, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_world_region_setters, "world region setters replace world and regions")
{
    ft_vector<int> region_ids;
    region_ids.push_back(6);
    region_ids.push_back(7);
    ft_world_region world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize(3, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());

    world.set_world_id(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    region_ids.clear();
    region_ids.push_back(20);
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
