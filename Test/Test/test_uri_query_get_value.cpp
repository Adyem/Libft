#include "../test_internal.hpp"
#include "../../Modules/URI/uri.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_uri_query_get_value_decodes_value)
{
    char *query_value;
    const char *query_string;

    query_string = "name=Ada%20Lovelace&empty=&count=1";
    query_value = uri_query_get_value(query_string,
            ft_strlen_size_t(query_string), "name", 4);
    FT_ASSERT(query_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(query_value, "Ada Lovelace"));
    cma_free(query_value);
    query_string = "flag&name=ada";
    query_value = uri_query_get_value(query_string,
            ft_strlen_size_t(query_string), "flag", 4);
    FT_ASSERT(query_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(query_value, ""));
    cma_free(query_value);
    query_value = uri_query_get_value("name=ada", 8, "missing", 7);
    FT_ASSERT(query_value == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, uri_get_error());
    return (1);
}
