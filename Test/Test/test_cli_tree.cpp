#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"

static void reset_option(cli_option *option, const char *long_name, char short_name,
    uint8_t type)
{
    option->long_name = long_name;
    option->short_name = short_name;
    option->type = type;
    option->required = FT_FALSE;
    option->env_name = ft_nullptr;
    option->help = "";
    option->default_value = ft_nullptr;
    option->present = FT_FALSE;
    option->value_string = ft_nullptr;
    option->value_string_storage = ft_nullptr;
    option->value_bool = FT_FALSE;
    option->value_int64 = 0;
    option->value_uint64 = 0U;
    option->value_double = 0.0;
    return ;
}

static void reset_command(cli_command *command, const char *name,
    cli_option *options, ft_size_t option_count)
{
    command->name = name;
    command->description = "";
    command->options = options;
    command->option_count = option_count;
    command->subcommands = ft_nullptr;
    command->subcommand_count = 0;
    command->positionals = ft_nullptr;
    command->positional_capacity = 0;
    command->positional_count = 0;
    command->selected_subcommand = ft_nullptr;
    command->help_requested = FT_FALSE;
    return ;
}

FT_TEST(test_cli_parse_nested_subcommand_tree)
{
    cli_option root_options[1];
    cli_option serve_options[1];
    cli_option status_options[1];
    cli_command command;
    cli_command serve_commands[1];
    cli_command status_commands[1];
    const char *arguments[6];

    reset_option(root_options + 0, "verbose", 'v', CLI_OPTION_BOOL);
    reset_option(serve_options + 0, "port", 'p', CLI_OPTION_UINT64);
    reset_option(status_options + 0, "detail", 'd', CLI_OPTION_BOOL);
    reset_command(&command, "tool", root_options, 1);
    reset_command(serve_commands + 0, "serve", serve_options, 1);
    reset_command(status_commands + 0, "status", status_options, 1);
    serve_commands[0].subcommands = status_commands;
    serve_commands[0].subcommand_count = 1;
    command.subcommands = serve_commands;
    command.subcommand_count = 1;
    arguments[0] = "tool";
    arguments[1] = "serve";
    arguments[2] = "--port";
    arguments[3] = "8080";
    arguments[4] = "status";
    arguments[5] = "--detail";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_parse(&command, 6, arguments));
    FT_ASSERT_EQ(serve_commands + 0, command.selected_subcommand);
    FT_ASSERT_EQ(status_commands + 0, serve_commands[0].selected_subcommand);
    FT_ASSERT_EQ(static_cast<uint64_t>(8080U), serve_options[0].value_uint64);
    FT_ASSERT_EQ(FT_TRUE, status_options[0].value_bool);
    return (1);
}

FT_TEST(test_cli_validate_rejects_duplicate_subcommand_names)
{
    cli_command command;
    cli_command subcommands[2];

    reset_command(&command, "tool", ft_nullptr, 0);
    reset_command(subcommands + 0, "serve", ft_nullptr, 0);
    reset_command(subcommands + 1, "serve", ft_nullptr, 0);
    command.subcommands = subcommands;
    command.subcommand_count = 2;
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, cli_validate(&command));
    return (1);
}
