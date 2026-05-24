#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void reset_cli_bool_option(cli_option *option)
{
    option->long_name = "verbose";
    option->short_name = 'v';
    option->type = CLI_OPTION_BOOL;
    option->required = FT_FALSE;
    option->env_name = ft_nullptr;
    option->help = "";
    option->default_value = ft_nullptr;
    option->present = FT_FALSE;
    option->value_string = ft_nullptr;
    option->value_bool = FT_FALSE;
    option->value_int64 = 0;
    option->value_uint64 = 0U;
    option->value_double = 0.0;
    return ;
}

static void reset_cli_bool_command(cli_command *command, cli_option *option)
{
    command->name = "tool";
    command->description = "";
    command->options = option;
    command->option_count = 1;
    command->subcommands = ft_nullptr;
    command->subcommand_count = 0;
    command->positionals = ft_nullptr;
    command->positional_capacity = 0;
    command->positional_count = 0;
    command->selected_subcommand = ft_nullptr;
    command->help_requested = FT_FALSE;
    return ;
}

FT_TEST(test_cli_get_bool_returns_parsed_value)
{
    cli_option option;
    cli_command command;
    const char *arguments[2];
    ft_bool verbose_value;

    reset_cli_bool_option(&option);
    reset_cli_bool_command(&command, &option);
    arguments[0] = "tool";
    arguments[1] = "--verbose";
    verbose_value = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_parse(&command, 2, arguments));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_get_bool(&command, "verbose",
        &verbose_value));
    FT_ASSERT_EQ(FT_TRUE, verbose_value);
    return (1);
}
