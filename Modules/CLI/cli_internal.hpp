#ifndef CLI_INTERNAL_HPP
# define CLI_INTERNAL_HPP

#include "cli.hpp"

int32_t cli_apply_option_value(cli_option *option, const char *value) noexcept;
cli_command *cli_find_subcommand(cli_command *command, const char *name) noexcept;
const cli_command *cli_find_subcommand(const cli_command *command,
    const char *name) noexcept;
int32_t cli_apply_defaults_recursive(cli_command *command) noexcept;

const cli_option *cli_get_present_option(const cli_command *command,
    const char *long_name, uint8_t expected_type, int32_t *error_code) noexcept;

#endif
