#include "../test_internal.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_region_definition_defaults, "Region definition defaults to zeroed values")
{
    ft_region_definition region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(0, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(ft_string(), region.get_name());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(ft_string(), region.get_description());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(0, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_initialize_with_fields_records_fields, "Region definition initialize stores provided values")
{
    ft_region_definition region;

    ft_string canyon;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, canyon.initialize("canyon"));
    ft_string rocky_pass;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rocky_pass.initialize("rocky pass"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(9, canyon,
        rocky_pass, 14));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(9, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("canyon", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("rocky pass", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(14, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_setters_update_fields, "Region definition setters overwrite stored data")
{
    ft_region_definition region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_region_id(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string spire;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spire.initialize("spire"));
    region.set_name(spire);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    ft_string towering_peak;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, towering_peak.initialize("towering peak"));
    region.set_description(towering_peak);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    region.set_recommended_level(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(5, region.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("spire", region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_STR_EQ("towering peak", region.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    FT_ASSERT_EQ(20, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_initialize_copy_restores_values, "Region definition initialize(copy) duplicates source values")
{
    ft_region_definition source;
    ft_region_definition clone;

    ft_string reef;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reef.initialize("reef"));
    ft_string shallow_waters;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shallow_waters.initialize("shallow waters"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, reef,
        shallow_waters, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(7, clone.get_region_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_STR_EQ("reef", clone.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_STR_EQ("shallow waters", clone.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    FT_ASSERT_EQ(6, clone.get_recommended_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}
