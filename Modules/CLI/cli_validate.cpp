#include "cli_internal.hpp"

static ft_bool cli_is_empty_string(const char *string)
{
    if (string == ft_nullptr)
    {
        return (FT_TRUE);
    }
    if (string[0] == '\0')
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static ft_bool cli_strings_equal(const char *left, const char *right)
{
    ft_size_t index;

    if (left == ft_nullptr || right == ft_nullptr)
    {
        return (FT_FALSE);
    }
    index = 0;
    while (left[index] != '\0' && right[index] != '\0')
    {
        if (left[index] != right[index])
        {
            return (FT_FALSE);
        }
        index++;
    }
    if (left[index] == right[index])
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static ft_bool cli_validate_option_type(uint8_t type)
{
    if (type <= CLI_OPTION_DOUBLE)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static int32_t cli_validate_option_list(const cli_command *command)
{
    ft_size_t index;
    ft_size_t other_index;

    if (command->option_count > 0 && command->options == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    index = 0;
    while (index < command->option_count)
    {
        if (cli_is_empty_string(command->options[index].long_name) == FT_TRUE)
        {
            return (FT_ERR_INVALID_ARGUMENT);
        }
        if (cli_validate_option_type(command->options[index].type) == FT_FALSE)
        {
            return (FT_ERR_UNSUPPORTED_TYPE);
        }
        other_index = 0;
        while (other_index < index)
        {
            if (cli_strings_equal(command->options[index].long_name,
                    command->options[other_index].long_name) == FT_TRUE)
            {
                return (FT_ERR_ALREADY_EXISTS);
            }
            if (command->options[index].short_name != '\0'
                && command->options[index].short_name
                    == command->options[other_index].short_name)
            {
                return (FT_ERR_ALREADY_EXISTS);
            }
            other_index++;
        }
        index++;
    }
    return (FT_ERR_SUCCESS);
}

static ft_bool cli_command_is_in_subcommands(const cli_command *command,
    const cli_command *candidate)
{
    ft_size_t index;

    if (command == ft_nullptr || candidate == ft_nullptr)
    {
        return (FT_FALSE);
    }
    index = 0;
    while (index < command->subcommand_count)
    {
        if (command->subcommands + index == candidate)
        {
            return (FT_TRUE);
        }
        index++;
    }
    return (FT_FALSE);
}

static int32_t cli_validate_subcommand_list(const cli_command *command)
{
    ft_size_t index;
    ft_size_t other_index;
    int32_t error_code;

    if (command->subcommand_count > 0 && command->subcommands == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    index = 0;
    while (index < command->subcommand_count)
    {
        if (cli_is_empty_string(command->subcommands[index].name) == FT_TRUE)
        {
            return (FT_ERR_INVALID_ARGUMENT);
        }
        other_index = 0;
        while (other_index < index)
        {
            if (cli_strings_equal(command->subcommands[index].name,
                    command->subcommands[other_index].name) == FT_TRUE)
            {
                return (FT_ERR_ALREADY_EXISTS);
            }
            other_index++;
        }
        error_code = cli_validate(command->subcommands + index);
        if (error_code != FT_ERR_SUCCESS)
        {
            return (error_code);
        }
        index++;
    }
    if (command->selected_subcommand != ft_nullptr
        && cli_command_is_in_subcommands(command, command->selected_subcommand)
            == FT_FALSE)
    {
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t cli_validate_command_tree(const cli_command *command)
{
    int32_t error_code;

    if (command == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (cli_is_empty_string(command->name) == FT_TRUE)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (command->positional_capacity > 0 && command->positionals == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    error_code = cli_validate_option_list(command);
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    return (cli_validate_subcommand_list(command));
}

int32_t cli_validate(const cli_command *command)
{
    return (cli_validate_command_tree(command));
}
