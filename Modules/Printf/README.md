# Printf

The `Printf` module provides printf-compatible formatting to stdout, file descriptors, fixed buffers, and `FILE *` streams. It also exposes a low-level formatting engine and custom specifier registration.

## Hooks and Formatter Types

- `t_pf_tmpfile_function` - Test hook type used to replace temporary-file creation.
- `t_pf_fflush_function` - Test hook type used to replace `fflush`.
- `t_pf_ftell_function` - Test hook type used to replace `ftell`.
- `t_pf_custom_formatter` - Custom specifier callback that receives a `va_list`, output string, and caller context.
- `t_pf_engine_write_callback` - Low-level writer callback used by the formatting engine.

## Public API

- `pf_printf(const char *format, ...)` - Formats to standard output.
- `pf_printf_fd(int32_t file_descriptor, const char *format, ...)` - Formats to a file descriptor.
- `pf_snprintf(char *string, ft_size_t size, const char *format, ...)` - Formats into a bounded caller buffer.
- `pf_vsnprintf(char *string, ft_size_t size, const char *format, va_list argument_list)` - `va_list` version of `pf_snprintf`.
- `ft_vfprintf(FILE *stream, const char *format, va_list argument_list)` - Formats to a C stream from a `va_list`.
- `ft_fprintf(FILE *stream, const char *format, ...)` - Formats to a C stream.
- `pf_set_tmpfile_function(...)` / `pf_reset_tmpfile_function()` - Install or reset the temp-file hook.
- `pf_set_fflush_function(...)` / `pf_reset_fflush_function()` - Install or reset the flush hook.
- `pf_set_ftell_function(...)` / `pf_reset_ftell_function()` - Install or reset the tell hook.
- `pf_register_custom_specifier(...)` - Registers a custom format specifier.
- `pf_unregister_custom_specifier(...)` - Removes a custom format specifier.
- `pf_enable_thread_safety()` / `pf_disable_thread_safety()` - Enable or disable formatter-global synchronization.
- `pf_engine_format(...)` - Runs the formatting engine and writes output through a callback.
