#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cli_get_double_returns_parsed_value)
{
    cli_option option;
    cli_command command;
    const char *arguments[3];
    double ratio_value;

    option = {"ratio", 'r', CLI_OPTION_DOUBLE, FT_FALSE, ft_nullptr, "",
        ft_nullptr, FT_FALSE, ft_nullptr, ft_nullptr, FT_FALSE, 0, 0U, 0.0};
    command = {"tool", "", &option, 1, ft_nullptr, 0, ft_nullptr, 0, 0,
        ft_nullptr, FT_FALSE};
    arguments[0] = "tool";
    arguments[1] = "--ratio";
    arguments[2] = "2.5";
    ratio_value = 0.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_parse(&command, 3, arguments));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_get_double(&command, "ratio", &ratio_value));
    FT_ASSERT(ratio_value > 2.49);
    FT_ASSERT(ratio_value < 2.51);
    return (1);
}
