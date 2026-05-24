#ifndef CLI_INTERNAL_HPP
# define CLI_INTERNAL_HPP

#include "cli.hpp"

const cli_option *cli_get_present_option(const cli_command *command,
    const char *long_name, uint8_t expected_type, int32_t *error_code) noexcept;

#endif
