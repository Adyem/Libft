#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cli_get_int64_returns_parsed_value)
{
    cli_option option;
    cli_command command;
    const char *arguments[3];
    int64_t count_value;

    option = {"count", 'c', CLI_OPTION_INT64, FT_FALSE, ft_nullptr, "",
        ft_nullptr, FT_FALSE, ft_nullptr, ft_nullptr, FT_FALSE, 0, 0U, 0.0};
    command = {"tool", "", &option, 1, ft_nullptr, 0, ft_nullptr, 0, 0,
        ft_nullptr, FT_FALSE};
    arguments[0] = "tool";
    arguments[1] = "--count";
    arguments[2] = "-42";
    count_value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_parse(&command, 3, arguments));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_get_int64(&command, "count", &count_value));
    FT_ASSERT_EQ(static_cast<int64_t>(-42), count_value);
    return (1);
}
