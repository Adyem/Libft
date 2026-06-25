#include "cli_internal.hpp"
#include "../Basic/class_nullptr.hpp"

int32_t cli_get_uint64(const cli_command *command, const char *long_name,
    uint64_t *value_out) noexcept
{
    const cli_option *option;
    int32_t error_code;

    if (value_out == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    option = cli_get_present_option(command, long_name, CLI_OPTION_UINT64,
            &error_code);
    if (option == ft_nullptr)
        return (error_code);
    *value_out = option->value_uint64;
    return (FT_ERR_SUCCESS);
}
