#include "../../JSon/json.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <string>

FT_TEST(test_json_read_from_string_null_input_sets_errno, "json reader rejects null input")
{
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(ft_nullptr);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_quote_sets_errno, "json reader detects unterminated strings")
{
    const char *content = "{ \"config\": { \"name\": \"value } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_colon_sets_errno, "json reader detects missing delimiters")
{
    const char *content = "{ \"config\" { \"name\": \"value\" } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_closing_sets_errno, "json reader requires closing braces")
{
    const char *content = "{ \"config\": { \"name\": \"value\" }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_fraction_digits_sets_errno, "json reader rejects missing fraction digits")
{
    const char *content = "{ \"config\": { \"value\": 1. } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_exponent_digits_sets_errno, "json reader rejects missing exponent digits")
{
    const char *content = "{ \"config\": { \"value\": 1e } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_success_resets_errno, "json reader clears errno on success")
{
    const char *content = "{ \"config\": { \"name\": \"value\" } }";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    json_free_groups(groups);
    return (1);
}


FT_TEST(test_json_read_from_string_decodes_escaped_strings, "json reader decodes escaped and unicode sequences")
{
    std::string content_string = "{ \"config\": { \"value\": \"";
    content_string.append("Line\\nBreak \\\"Quote\\\" Backslash\\\\ Unicode ");
    content_string.append("\\u263A ");
    content_string.append("\\uD834\\uDD1E");
    content_string.append("\" } }");
    ft_errno = FT_ERR_INVALID_ARGUMENT;
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
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    json_free_groups(groups);
    return (1);
}

FT_TEST(test_json_read_from_file_missing_file_sets_errno, "json reader reports io errors")
{
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_file("Test/nonexistent_json_reader.json");
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}
