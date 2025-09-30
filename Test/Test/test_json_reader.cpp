#include "../../JSon/json.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_json_read_from_string_null_input_sets_errno, "json reader rejects null input")
{
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(ft_nullptr);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_quote_sets_errno, "json reader detects unterminated strings")
{
    const char *content = "{ \"config\": { \"name\": \"value } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_colon_sets_errno, "json reader detects missing delimiters")
{
    const char *content = "{ \"config\" { \"name\": \"value\" } }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_missing_closing_sets_errno, "json reader requires closing braces")
{
    const char *content = "{ \"config\": { \"name\": \"value\" }";
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_json_read_from_string_success_resets_errno, "json reader clears errno on success")
{
    const char *content = "{ \"config\": { \"name\": \"value\" } }";
    ft_errno = FT_EINVAL;
    json_group *groups = json_read_from_string(content);
    FT_ASSERT(groups != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    json_free_groups(groups);
    return (1);
}

FT_TEST(test_json_read_from_file_missing_file_sets_errno, "json reader reports io errors")
{
    ft_errno = ER_SUCCESS;
    json_group *groups = json_read_from_file("Test/nonexistent_json_reader.json");
    FT_ASSERT(groups == ft_nullptr);
    FT_ASSERT_EQ(FT_EIO, ft_errno);
    return (1);
}
