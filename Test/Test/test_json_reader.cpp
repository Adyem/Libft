#include "../test_internal.hpp"
#include "../../Modules/JSon/json.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <string>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_json_read_from_string_null_input_sets_errno)
{
    json_group *groups = json_read_from_string(ft_nullptr);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_quote_sets_errno)
{
    const char *content = "{ \"config\": { \"name\": \"value } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_colon_sets_errno)
{
    const char *content = "{ \"config\" { \"name\": \"value\" } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_closing_sets_errno)
{
    const char *content = "{ \"config\": { \"name\": \"value\" }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_fraction_digits_sets_errno)
{
    const char *content = "{ \"config\": { \"value\": 1. } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_exponent_digits_sets_errno)
{
    const char *content = "{ \"config\": { \"value\": 1e } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}

FT_TEST(test_json_read_from_string_success_resets_errno)
{
    const char *content = "{ \"config\": { \"name\": \"value\" } }";
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups != ft_nullptr);
    json_free_groups(groups);
    return (1);
}

FT_TEST(test_json_read_from_string_decodes_escaped_strings)
{
    std::string content_string = "{ \"config\": { \"value\": \"";
    content_string.append("Line\\nBreak \\\"Quote\\\" Backslash\\\\ Unicode ");
    content_string.append("\\u263A ");
    content_string.append("\\uD834\\uDD1E");
    content_string.append("\" } }");
    json_group *groups = json_read_from_string(content_string.c_str());
    FT_ASSERT(groups != ft_nullptr);
    json_group *group = json_find_group(groups, "config");
    FT_ASSERT(group != ft_nullptr);
    json_item *item = json_find_item(group, "value");
    FT_ASSERT(item != ft_nullptr);
    FT_ASSERT(item->value != ft_nullptr);
    std::string expected = "Line";
    expected.push_back('\n');
    expected.append("Break \"Quote\" Backslash\\ Unicode ");
    expected.append("\xE2\x98\xBA ");
    expected.append("\xF0\x9D\x84\x9E");
    std::string actual = item->value;
    FT_ASSERT_EQ(expected, actual);
    json_free_groups(groups);
    return (1);
}

FT_TEST(test_json_read_from_file_missing_file_sets_errno)
{
    json_group *groups = json_read_from_file("Test/nonexistent_json_reader.json");
    FT_ASSERT(groups == ft_nullptr);
    return (1);
}
