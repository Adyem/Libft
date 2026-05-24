#ifndef CLI_HPP
#define CLI_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

enum cli_option_type : uint8_t
{
    CLI_OPTION_BOOL = 0,
    CLI_OPTION_STRING = 1,
    CLI_OPTION_INT64 = 2,
    CLI_OPTION_UINT64 = 3,
    CLI_OPTION_DOUBLE = 4
};

struct cli_option
{
    const char          *long_name;
    char                short_name;
    uint8_t             type;
    ft_bool             required;
    const char          *env_name;
    const char          *help;
    const char          *default_value;
    ft_bool             present;
    const char          *value_string;
    ft_bool             value_bool;
    int64_t             value_int64;
    uint64_t            value_uint64;
    double              value_double;
};

struct cli_command
{
    const char          *name;
    const char          *description;
    cli_option       *options;
    ft_size_t           option_count;
    cli_command      *subcommands;
    ft_size_t           subcommand_count;
    const char          **positionals;
    ft_size_t           positional_capacity;
    ft_size_t           positional_count;
    cli_command      *selected_subcommand;
    ft_bool             help_requested;
};

int32_t         cli_parse(cli_command *command, ft_size_t argument_count,
                    const char *const *argument_values);
cli_option   *cli_find_option(cli_command *command, const char *long_name);
const cli_option *cli_find_option_const(const cli_command *command,
                    const char *long_name);
int32_t         cli_get_bool(const cli_command *command, const char *long_name,
                    ft_bool *value_out) noexcept;
int32_t         cli_get_string(const cli_command *command, const char *long_name,
                    const char **value_out) noexcept;
int32_t         cli_get_int64(const cli_command *command, const char *long_name,
                    int64_t *value_out) noexcept;
int32_t         cli_get_uint64(const cli_command *command, const char *long_name,
                    uint64_t *value_out) noexcept;
int32_t         cli_get_double(const cli_command *command, const char *long_name,
                    double *value_out) noexcept;
int32_t         cli_format_help(const cli_command *command,
                    const char *program_name, char *buffer, ft_size_t buffer_size);

#endif
