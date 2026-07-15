#include "../test_internal.hpp"
#include "../../Modules/URI/uri.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_uri_query_has_key_accepts_empty_and_encoded_values)
{
    const char *query_string;

    query_string = "empty=&name=Ada%20Lovelace";
    FT_ASSERT_EQ(FT_TRUE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "empty", 5));
    FT_ASSERT_EQ(FT_TRUE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "name", 4));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "missing", 7));
    return (1);
}

FT_TEST(test_uri_query_has_key_rejects_invalid_null_spans_and_prefixes)
{
    const char *query_string;

    query_string = "alpha=1&alphabet=2&name=Ada+Lovelace";
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "alph", 4));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "missing", 7));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key(ft_nullptr, 1, "alpha", 5));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), ft_nullptr, 1));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key("bad=%ZZ", 7, "bad", 3));
    return (1);
}

FT_TEST(test_uri_query_has_key_matches_encoded_keys_and_duplicate_first_value)
{
    const char *query_string;

    query_string = "a%20b=first&a%20b=second";
    FT_ASSERT_EQ(FT_TRUE, uri_query_has_key(query_string,
            ft_strlen_size_t(query_string), "a b", 3));
    FT_ASSERT_EQ(FT_TRUE, uri_query_has_key("flag&&tail=value",
            ft_strlen_size_t("flag&&tail=value"), "flag", 4));
    FT_ASSERT_EQ(FT_FALSE, uri_query_has_key("", 0, "flag", 4));
    return (1);
}
