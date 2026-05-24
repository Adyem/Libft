#include "cli_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"

int32_t cli_get_string(const cli_command *command, const char *long_name,
    const char **value_out) noexcept
{
    const cli_option *option;
    int32_t error_code;

    if (value_out == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    option = cli_get_present_option(command, long_name, CLI_OPTION_STRING,
            &error_code);
    if (option == ft_nullptr)
        return (error_code);
    *value_out = option->value_string;
    return (FT_ERR_SUCCESS);
}
