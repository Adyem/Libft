#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_region_definition_basic_accessors, "Region definition accessors return stored values")
{
    ft_region_definition region;

    ft_string spire;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spire.initialize("spire"));
    ft_string towering_peak;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, towering_peak.initialize("towering peak"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(5, spire,
        towering_peak, 22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());

    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("spire", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("towering peak", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(22, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_resize_fields, "Region definition setters update stored state")
{
    ft_region_definition region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string reef;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reef.initialize("reef"));
    region.set_name(reef);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string shallow_waters;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shallow_waters.initialize("shallow waters"));
    region.set_description(shallow_waters);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(9, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("reef", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("shallow waters", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(6, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_initialize_copy, "Region definition initialize(copy) duplicates state")
{
    ft_region_definition source;
    ft_region_definition clone;

    ft_string delta;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, delta.initialize("delta"));
    ft_string river_maze;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, river_maze.initialize("river maze"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, delta,
        river_maze, 8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(3, clone.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_STR_EQ("delta", clone.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_STR_EQ("river maze", clone.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(8, clone.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}
