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

FT_TEST(test_cli_format_help_includes_rich_metadata)
{
    cli_option options[1];
    cli_command command;
    cli_command subcommands[1];
    char buffer[512];

    reset_option(options + 0, "verbose", 'v', CLI_OPTION_BOOL);
    options[0].required = FT_TRUE;
    options[0].env_name = "VERBOSE";
    options[0].help = "enable verbose output";
    options[0].default_value = "false";
    reset_command(&command, "tool", options, 1);
    command.description = "demo tool";
    reset_command(subcommands + 0, "serve", ft_nullptr, 0);
    subcommands[0].description = "run server";
    command.subcommands = subcommands;
    command.subcommand_count = 1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cli_format_help(&command, "tool", buffer, 512));
    FT_ASSERT(std::strstr(buffer, "Usage: tool <command> [options]") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "Commands:") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "- serve - run server") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "--verbose [bool]") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "[required]") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "[env: VERBOSE]") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "[default: false]") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, "enable verbose output") != ft_nullptr);
    return (1);
}
