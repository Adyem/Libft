#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cli_get_string_returns_parsed_value)
{
    cli_option option;
    cli_command command;
    const char *arguments[3];
    const char *name_value;

    option = {"name", 'n', CLI_OPTION_STRING, FT_FALSE, ft_nullptr, "",
        ft_nullptr, FT_FALSE, ft_nullptr, ft_nullptr, FT_FALSE, 0, 0U, 0.0};
    command = {"tool", "", &option, 1, ft_nullptr, 0, ft_nullptr, 0, 0,
        ft_nullptr, FT_FALSE};
    arguments[0] = "tool";
    arguments[1] = "--name";
    arguments[2] = "demo";
    name_value = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_parse(&command, 3, arguments));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_get_string(&command, "name", &name_value));
    FT_ASSERT_EQ(0, ft_strcmp(name_value, "demo"));
    return (1);
}
