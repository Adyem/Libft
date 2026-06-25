#include "cli_internal.hpp"
#include "../Basic/class_nullptr.hpp"

const cli_option *cli_get_present_option(const cli_command *command,
    const char *long_name, uint8_t expected_type, int32_t *error_code) noexcept
{
    const cli_option *option;

    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESS;
    if (command == ft_nullptr || long_name == ft_nullptr)
    {
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_INVALID_POINTER;
        return (ft_nullptr);
    }
    option = cli_find_option_const(command, long_name);
    if (option == ft_nullptr || option->present == FT_FALSE)
    {
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_NOT_FOUND;
        return (ft_nullptr);
    }
    if (option->type != expected_type)
    {
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_UNSUPPORTED_TYPE;
        return (ft_nullptr);
    }
    return (option);
}
