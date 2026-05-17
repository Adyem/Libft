#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_big_number.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Template/move.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

#define INIT_BIG_NUMBER(number) FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize())

FT_TEST(test_big_number_hex_serialization_round_trip)
{
    ft_big_number decimal_value;
    INIT_BIG_NUMBER(decimal_value);

    decimal_value.assign("3735928559");

    ft_string *hex_string = big_number_to_hex_string(decimal_value);
    FT_ASSERT(hex_string != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, hex_string->get_error());
    FT_ASSERT(*hex_string == "DEADBEEF");

    ft_big_number *parsed_value = big_number_from_hex_string(hex_string->c_str());

    FT_ASSERT(parsed_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(parsed_value->c_str(), decimal_value.c_str()));
    FT_ASSERT(!parsed_value->is_negative());
    FT_ASSERT(parsed_value->is_positive());
    (void)parsed_value->destroy();
    delete parsed_value;
    (void)hex_string->destroy();
    delete hex_string;
    return (1);
}

FT_TEST(test_big_number_hex_serialization_prefix_support)
{
    ft_big_number negative_decimal;
    INIT_BIG_NUMBER(negative_decimal);

    negative_decimal.assign("-4660");

    ft_string *negative_hex = big_number_to_hex_string(negative_decimal);
    FT_ASSERT(negative_hex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, negative_hex->get_error());
    FT_ASSERT(*negative_hex == "-1234");

    ft_big_number *round_trip_negative = big_number_from_hex_string(negative_hex->c_str());

    FT_ASSERT(round_trip_negative != ft_nullptr);
    FT_ASSERT(round_trip_negative->is_negative());
    FT_ASSERT_EQ(0, std::strcmp(round_trip_negative->c_str(), "4660"));

    ft_big_number *prefixed_negative = big_number_from_hex_string("-0x1234");

    FT_ASSERT(prefixed_negative != ft_nullptr);
    FT_ASSERT(prefixed_negative->is_negative());
    FT_ASSERT_EQ(0, std::strcmp(prefixed_negative->c_str(), "4660"));

    ft_big_number *prefixed_positive = big_number_from_hex_string("0XCAFEBABE");

    FT_ASSERT(prefixed_positive != ft_nullptr);
    FT_ASSERT(!prefixed_positive->is_negative());
    FT_ASSERT(prefixed_positive->is_positive());
    FT_ASSERT_EQ(0, std::strcmp(prefixed_positive->c_str(), "3405691582"));
    (void)prefixed_positive->destroy();
    delete prefixed_positive;
    (void)prefixed_negative->destroy();
    delete prefixed_negative;
    (void)round_trip_negative->destroy();
    delete round_trip_negative;
    (void)negative_hex->destroy();
    delete negative_hex;
    return (1);
}

#undef INIT_BIG_NUMBER
