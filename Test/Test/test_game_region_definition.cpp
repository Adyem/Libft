#include "../../Game/ft_region_definition.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/move.hpp"

FT_TEST(test_game_region_definition_defaults, "Region definition initializes default values")
{
    ft_region_definition region;

    FT_ASSERT_EQ(0, region.get_region_id());
    FT_ASSERT_EQ(ft_string(), region.get_name());
    FT_ASSERT_EQ(ft_string(), region.get_description());
    FT_ASSERT_EQ(0, region.get_recommended_level());
    FT_ASSERT_EQ(ER_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_parameterized_constructor_sets_fields, "Region definition constructor stores provided details")
{
    ft_region_definition region(3, ft_string("forest"), ft_string("dense woods"), 5);

    FT_ASSERT_EQ(3, region.get_region_id());
    FT_ASSERT_EQ(ft_string("forest"), region.get_name());
    FT_ASSERT_EQ(ft_string("dense woods"), region.get_description());
    FT_ASSERT_EQ(5, region.get_recommended_level());
    FT_ASSERT_EQ(ER_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_setters_update_fields, "Region definition setters overwrite stored values")
{
    ft_region_definition region;

    region.set_region_id(7);
    region.set_name(ft_string("cove"));
    region.set_description(ft_string("hidden bay"));
    region.set_recommended_level(9);
    FT_ASSERT_EQ(7, region.get_region_id());
    FT_ASSERT_EQ(ft_string("cove"), region.get_name());
    FT_ASSERT_EQ(ft_string("hidden bay"), region.get_description());
    FT_ASSERT_EQ(9, region.get_recommended_level());
    FT_ASSERT_EQ(ER_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_copy_constructor_clones_state, "Region definition copy constructor duplicates values")
{
    ft_region_definition original(12, ft_string("harbor"), ft_string("busy docks"), 4);

    ft_region_definition copy(original);
    FT_ASSERT_EQ(12, copy.get_region_id());
    FT_ASSERT_EQ(ft_string("harbor"), copy.get_name());
    FT_ASSERT_EQ(ft_string("busy docks"), copy.get_description());
    FT_ASSERT_EQ(4, copy.get_recommended_level());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, original.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_move_constructor_transfers_state, "Region definition move constructor transfers and resets source")
{
    ft_region_definition source(15, ft_string("ruins"), ft_string("crumbling stones"), 11);

    ft_region_definition moved(ft_move(source));
    FT_ASSERT_EQ(15, moved.get_region_id());
    FT_ASSERT_EQ(ft_string("ruins"), moved.get_name());
    FT_ASSERT_EQ(ft_string("crumbling stones"), moved.get_description());
    FT_ASSERT_EQ(11, moved.get_recommended_level());
    FT_ASSERT_EQ(0, source.get_region_id());
    FT_ASSERT_EQ(ft_string(), source.get_name());
    FT_ASSERT_EQ(ft_string(), source.get_description());
    FT_ASSERT_EQ(0, source.get_recommended_level());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}
