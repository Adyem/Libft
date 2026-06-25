#include "../test_internal.hpp"
#include "../../Modules/URI/uri.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_uri_parse_absolute_uri)
{
    uri_components components;
    const char *uri_input;

    uri_input = "https://user@example.com:8443/a/b?count=1#fragment";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, uri_parse(uri_input,
        ft_strlen_size_t(uri_input), &components));
    FT_ASSERT_EQ(0, ft_strcmp(components.scheme, "https"));
    FT_ASSERT_EQ(0, ft_strcmp(components.userinfo, "user"));
    FT_ASSERT_EQ(0, ft_strcmp(components.host, "example.com"));
    FT_ASSERT_EQ(0, ft_strcmp(components.port, "8443"));
    FT_ASSERT_EQ(0, ft_strcmp(components.path, "/a/b"));
    FT_ASSERT_EQ(0, ft_strcmp(components.query, "count=1"));
    FT_ASSERT_EQ(0, ft_strcmp(components.fragment, "fragment"));
    uri_components_destroy(&components);
    return (1);
}

FT_TEST(test_uri_parse_relative_reference)
{
    uri_components components;
    const char *uri_input;

    uri_input = "../asset.png?cache=false";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, uri_parse(uri_input,
        ft_strlen_size_t(uri_input), &components));
    FT_ASSERT(components.scheme == ft_nullptr);
    FT_ASSERT(components.host == ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(components.path, "../asset.png"));
    FT_ASSERT_EQ(0, ft_strcmp(components.query, "cache=false"));
    uri_components_destroy(&components);
    return (1);
}
