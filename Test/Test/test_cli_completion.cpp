#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstring>
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

FT_TEST(test_cli_generate_completion_script_includes_nested_paths)
{
    cli_option root_options[1];
    cli_command command;
    cli_command subcommands[1];
    cli_command nested_subcommands[1];
    char buffer[2048];

    reset_option(root_options + 0, "verbose", 'v', CLI_OPTION_BOOL);
    reset_command(&command, "tool", root_options, 1);
    reset_command(subcommands + 0, "serve", ft_nullptr, 0);
    reset_command(nested_subcommands + 0, "status", ft_nullptr, 0);
    subcommands[0].subcommands = nested_subcommands;
    subcommands[0].subcommand_count = 1;
    command.subcommands = subcommands;
    command.subcommand_count = 1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_generate_completion_script(&command, "tool",
            buffer, 2048));
    FT_ASSERT(std::strstr(buffer, "complete -F _tool_completion tool") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "\"serve\"") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "\"serve.status\"") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "--no-verbose") != ft_nullptr);
    return (1);
}
