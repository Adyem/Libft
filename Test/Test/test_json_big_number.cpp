#include "../../JSon/json.hpp"
#include "../../JSon/document.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_json_parse_detects_big_number, "json parser promotes oversized integers to big numbers")
{
    const char *content = "{ \"numbers\": { \"large\": 9223372036854775808 } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups != ft_nullptr);
    json_group *numbers_group = json_find_group(groups, "numbers");
    FT_ASSERT(numbers_group != ft_nullptr);
    json_item *large_item = json_find_item(numbers_group, "large");
    FT_ASSERT(large_item != ft_nullptr);
    FT_ASSERT(large_item->is_big_number == true);
    FT_ASSERT(large_item->big_number != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(large_item->value, "9223372036854775808"));
    FT_ASSERT_EQ(0, ft_strcmp(large_item->big_number->c_str(), "9223372036854775808"));
    json_free_groups(groups);
    return (1);
}

FT_TEST(test_json_big_number_roundtrip, "json serialization preserves large integers")
{
    const char *content = "{ \"numbers\": { \"massive\": 18446744073709551616 } }";
    json_document document;
    FT_ASSERT_EQ(0, document.read_from_string(content));
    json_group *numbers_group = document.find_group("numbers");
    FT_ASSERT(numbers_group != ft_nullptr);
    json_item *massive_item = document.find_item(numbers_group, "massive");
    FT_ASSERT(massive_item != ft_nullptr);
    FT_ASSERT(massive_item->is_big_number == true);
    FT_ASSERT(massive_item->big_number != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(massive_item->big_number->c_str(), "18446744073709551616"));
    char *serialized = document.write_to_string();
    FT_ASSERT(serialized != ft_nullptr);
    const char *expected = "{\n  \"numbers\": {\n    \"massive\": 18446744073709551616\n  }\n}\n";
    FT_ASSERT_EQ(0, ft_strcmp(expected, serialized));
    json_document roundtrip;
    FT_ASSERT_EQ(0, roundtrip.read_from_string(serialized));
    json_group *roundtrip_group = roundtrip.find_group("numbers");
    FT_ASSERT(roundtrip_group != ft_nullptr);
    json_item *roundtrip_item = roundtrip.find_item(roundtrip_group, "massive");
    FT_ASSERT(roundtrip_item != ft_nullptr);
    FT_ASSERT(roundtrip_item->is_big_number == true);
    FT_ASSERT(roundtrip_item->big_number != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(roundtrip_item->big_number->c_str(), "18446744073709551616"));
    cma_free(serialized);
    return (1);
}
