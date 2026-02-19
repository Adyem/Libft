#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_region_definition_basic_accessors, "Region definition accessors return stored values")
{
    ft_region_definition region(5, ft_string("spire"), ft_string("towering peak"), 22);

    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(ft_string("spire"), region.get_name());
    FT_ASSERT_EQ(ft_string("towering peak"), region.get_description());
    FT_ASSERT_EQ(22, region.get_recommended_level());
    return (1);
}

FT_TEST(test_game_region_definition_resize_fields, "Region definition setters update stored state")
{
    ft_region_definition region;

    region.set_region_id(9);
    region.set_name(ft_string("reef"));
    region.set_description(ft_string("shallow waters"));
    region.set_recommended_level(6);
    FT_ASSERT_EQ(9, region.get_region_id());
    FT_ASSERT_EQ(ft_string("reef"), region.get_name());
    FT_ASSERT_EQ(ft_string("shallow waters"), region.get_description());
    FT_ASSERT_EQ(6, region.get_recommended_level());
    return (1);
}

FT_TEST(test_game_region_definition_initialize_copy, "Region definition initialize(copy) duplicates state")
{
    ft_region_definition source(3, ft_string("delta"), ft_string("river maze"), 8);
    ft_region_definition clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(3, clone.get_region_id());
    FT_ASSERT_EQ(ft_string("delta"), clone.get_name());
    FT_ASSERT_EQ(ft_string("river maze"), clone.get_description());
    FT_ASSERT_EQ(8, clone.get_recommended_level());
    return (1);
}
