#include "../../Game/ft_region_definition.hpp"
#include "../../Template/move.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_region_definition_copy_assignment_overwrites_values, "Region definition copy assignment duplicates source state")
{
    ft_region_definition source(9, ft_string("canyon"), ft_string("rocky pass"), 14);
    ft_region_definition destination(2, ft_string("isle"), ft_string("remote"), 3);

    destination = source;
    FT_ASSERT_EQ(9, destination.get_region_id());
    FT_ASSERT_EQ(ft_string("canyon"), destination.get_name());
    FT_ASSERT_EQ(ft_string("rocky pass"), destination.get_description());
    FT_ASSERT_EQ(14, destination.get_recommended_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_move_assignment_transfers_state, "Region definition move assignment transfers and clears source")
{
    ft_region_definition source(5, ft_string("spire"), ft_string("towering peak"), 20);
    ft_region_definition destination(7, ft_string("reef"), ft_string("shallow waters"), 6);

    destination = ft_move(source);
    FT_ASSERT_EQ(5, destination.get_region_id());
    FT_ASSERT_EQ(ft_string("spire"), destination.get_name());
    FT_ASSERT_EQ(ft_string("towering peak"), destination.get_description());
    FT_ASSERT_EQ(20, destination.get_recommended_level());
    FT_ASSERT_EQ(0, source.get_region_id());
    FT_ASSERT_EQ(ft_string(), source.get_name());
    FT_ASSERT_EQ(ft_string(), source.get_description());
    FT_ASSERT_EQ(0, source.get_recommended_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_self_copy_assignment_no_change, "Self copy assignment keeps region definition intact")
{
    ft_region_definition region(4, ft_string("tundra"), ft_string("frozen wastes"), 16);

    region = region;
    FT_ASSERT_EQ(4, region.get_region_id());
    FT_ASSERT_EQ(ft_string("tundra"), region.get_name());
    FT_ASSERT_EQ(ft_string("frozen wastes"), region.get_description());
    FT_ASSERT_EQ(16, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_self_move_assignment_no_change, "Self move assignment preserves region definition state")
{
    ft_region_definition region(13, ft_string("delta"), ft_string("river maze"), 8);

    region = ft_move(region);
    FT_ASSERT_EQ(13, region.get_region_id());
    FT_ASSERT_EQ(ft_string("delta"), region.get_name());
    FT_ASSERT_EQ(ft_string("river maze"), region.get_description());
    FT_ASSERT_EQ(8, region.get_recommended_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_error_string_reports_success, "Region definition error string reflects success state")
{
    ft_region_definition region(1, ft_string("marsh"), ft_string("foggy wetland"), 2);

    FT_ASSERT_STR_EQ(ft_strerror(FT_ER_SUCCESSS), region.get_error_str());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    return (1);
}

FT_TEST(test_game_region_definition_get_region_id_sets_errno_success, "Region id getter resets errno to success")
{
    ft_region_definition region(10, ft_string("gorge"), ft_string("steep cliffs"), 12);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    FT_ASSERT_EQ(10, region.get_region_id());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_region_definition_get_name_sets_errno_success, "Region name getter clears errno to success")
{
    ft_region_definition region(21, ft_string("plateau"), ft_string("broad rise"), 7);

    ft_errno = FT_ERR_GAME_INVALID_MOVE;
    FT_ASSERT_EQ(ft_string("plateau"), region.get_name());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_region_definition_set_region_id_sets_errno_success, "Region id setter updates errno to success")
{
    ft_region_definition region;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    region.set_region_id(18);
    FT_ASSERT_EQ(18, region.get_region_id());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_region_definition_set_description_sets_errno_success, "Region description setter resets errno to success")
{
    ft_region_definition region;

    ft_errno = FT_ERR_MUTEX_NOT_OWNER;
    region.set_description(ft_string("ancient ruins"));
    FT_ASSERT_EQ(ft_string("ancient ruins"), region.get_description());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_region_definition_get_error_sets_errno_success, "Region definition get_error clears errno to success")
{
    ft_region_definition region(30, ft_string("crater"), ft_string("impact site"), 25);

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

