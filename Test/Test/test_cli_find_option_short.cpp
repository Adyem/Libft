#include "../test_internal.hpp"
#include "../../Modules/CLI/cli.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_cli_find_option_short_finds_declared_short_name)
{
    cli_option option;
    cli_command command;

    option.short_name = 'v';
    command.options = &option;
    command.option_count = 1U;
    command.selected_subcommand = ft_nullptr;
    FT_ASSERT(cli_find_option_short(&command, 'v') == &option);
    FT_ASSERT(cli_find_option_short(&command, 'x') == ft_nullptr);
    return (1);
}

FT_TEST(test_cli_find_option_short_searches_selected_subcommand)
{
    cli_option parent_option;
    cli_option child_option;
    cli_command child_command;
    cli_command parent_command;

    parent_option.short_name = 'p';
    child_option.short_name = 'c';
    child_command.options = &child_option;
    child_command.option_count = 1U;
    child_command.selected_subcommand = ft_nullptr;
    parent_command.options = &parent_option;
    parent_command.option_count = 1U;
    parent_command.selected_subcommand = &child_command;
    FT_ASSERT(cli_find_option_short(&parent_command, 'p') == &parent_option);
    FT_ASSERT(cli_find_option_short(&parent_command, 'c') == &child_option);
    return (1);
}

FT_TEST(test_cli_find_option_short_rejects_null_and_empty_names)
{
    cli_command command;

    command.options = ft_nullptr;
    command.option_count = 0U;
    command.selected_subcommand = ft_nullptr;
    FT_ASSERT(cli_find_option_short(ft_nullptr, 'x') == ft_nullptr);
    FT_ASSERT(cli_find_option_short(&command, '\0') == ft_nullptr);
    FT_ASSERT(cli_find_option_short(&command, 'x') == ft_nullptr);
    return (1);
}
