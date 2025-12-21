#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include <cstring>

FT_TEST(test_big_number_hex_serialization_round_trip, "ft_big_number hex serialization round trip")
{
    ft_big_number decimal_value;

    decimal_value.assign("3735928559");
    FT_ASSERT_EQ(0, decimal_value.get_error());

    ft_string hex_string = big_number_to_hex_string(decimal_value);
    FT_ASSERT_EQ(0, hex_string.get_error());
    FT_ASSERT(hex_string == "DEADBEEF");

    ft_big_number parsed_value = big_number_from_hex_string(hex_string.c_str());

    FT_ASSERT_EQ(0, parsed_value.get_error());
    FT_ASSERT_EQ(0, std::strcmp(parsed_value.c_str(), decimal_value.c_str()));
    FT_ASSERT(!parsed_value.is_negative());
    FT_ASSERT(parsed_value.is_positive());
    return (1);
}

FT_TEST(test_big_number_hex_serialization_prefix_support, "ft_big_number hex serialization handles prefixes and signs")
{
    ft_big_number negative_decimal;

    negative_decimal.assign("-4660");
    FT_ASSERT_EQ(0, negative_decimal.get_error());

    ft_string negative_hex = big_number_to_hex_string(negative_decimal);
    FT_ASSERT_EQ(0, negative_hex.get_error());
    FT_ASSERT(negative_hex == "-1234");

    ft_big_number round_trip_negative = big_number_from_hex_string(negative_hex.c_str());

    FT_ASSERT_EQ(0, round_trip_negative.get_error());
    FT_ASSERT(round_trip_negative.is_negative());
    FT_ASSERT_EQ(0, std::strcmp(round_trip_negative.c_str(), "4660"));

    ft_big_number prefixed_negative = big_number_from_hex_string("-0x1234");

    FT_ASSERT_EQ(0, prefixed_negative.get_error());
    FT_ASSERT(prefixed_negative.is_negative());
    FT_ASSERT_EQ(0, std::strcmp(prefixed_negative.c_str(), "4660"));

    ft_big_number prefixed_positive = big_number_from_hex_string("0XCAFEBABE");

    FT_ASSERT_EQ(0, prefixed_positive.get_error());
    FT_ASSERT(!prefixed_positive.is_negative());
    FT_ASSERT(prefixed_positive.is_positive());
    FT_ASSERT_EQ(0, std::strcmp(prefixed_positive.c_str(), "3405691582"));
    return (1);
}
