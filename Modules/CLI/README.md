# CLI

The `CLI` module parses command-line arguments into caller-defined command and option tables. It supports typed option values, required options, environment fallback names, recursive subcommand trees, positionals, config merging, completion generation, and richer help formatting.
Config text uses dot-separated command paths for nested options, and string values merged from config files are stored through module-owned string storage.

## Types

- `cli_option_type` - Enumerates supported option kinds: boolean, string, signed integer, unsigned integer, and double.
- `cli_option` - Describes one option and stores the parse result. Public fields include long and short names, type, required flag, environment variable name, help/default strings, present flag, typed value storage, and an internal string-storage slot used by config merging.
- `cli_command` - Describes a command. Public fields include name, description, option array, subcommand array, positional storage, selected subcommand, and help-request state.

## Public API

- `cli_parse(cli_command *command, ft_size_t argument_count, const char *const *argument_values)` - Parses an argv-style input array into a command tree.
- `cli_find_option(cli_command *command, const char *long_name)` - Finds a mutable option by long name.
- `cli_find_option_const(const cli_command *command, const char *long_name)` - Finds a read-only option by long name.
- `cli_get_bool(const cli_command *command, const char *long_name, ft_bool *value_out)` - Reads a parsed boolean option.
- `cli_get_string(const cli_command *command, const char *long_name, const char **value_out)` - Reads a parsed string option.
- `cli_get_int64(const cli_command *command, const char *long_name, int64_t *value_out)` - Reads a parsed signed integer option.
- `cli_get_uint64(const cli_command *command, const char *long_name, uint64_t *value_out)` - Reads a parsed unsigned integer option.
- `cli_get_double(const cli_command *command, const char *long_name, double *value_out)` - Reads a parsed double option.
- `cli_validate(const cli_command *command)` - Validates command names, option names, subcommand trees, and duplicate entries.
- `cli_format_help(const cli_command *command, const char *program_name, char *buffer, ft_size_t buffer_size)` - Writes command usage/help text into a caller-owned buffer.
- `cli_generate_completion_script(const cli_command *command, const char *program_name, char *buffer, ft_size_t buffer_size)` - Emits a Bash completion script for the command tree.
- `cli_merge_config_text(cli_command *command, const char *config_text)` - Applies `key=value` configuration lines to a command tree.
- `cli_merge_config_file(cli_command *command, const char *path)` - Reads a config file and merges it into a command tree.
