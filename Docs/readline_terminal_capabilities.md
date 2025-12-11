# ReadLine terminal capability detection

The ReadLine module relies on the Compatebility shims to adapt to the host
terminal. The helpers encapsulate the OS specific APIs and surface consistent
error handling via `ft_errno` so higher level code can react the same way on
Windows and POSIX environments.

## Raw mode toggles

`cmp_readline_enable_raw_mode()` switches the calling thread's standard input to
raw mode so keystrokes are delivered to ReadLine without the platform line
editor intervening. The function records the original console configuration and
returns `0` on success or `-1` on failure. On Windows the implementation calls
`GetConsoleMode`, strips `ENABLE_ECHO_INPUT` and `ENABLE_LINE_INPUT`, and
restores the previous mode in `cmp_readline_disable_raw_mode()` by invoking
`SetConsoleMode`. On POSIX systems the shims use `tcgetattr`/`tcsetattr` to
update `termios` flags and restore them when raw mode is no longer required.
Both variants map platform errors back through `ft_map_system_error` so callers
receive consistent `ft_errno` values.【F:Compatebility/Compatebility_readline.cpp†L1-L86】【F:Compatebility/Compatebility_readline.cpp†L87-L144】

Callers should pair every successful `cmp_readline_enable_raw_mode()` invocation
with `cmp_readline_disable_raw_mode()` in a `try`/`catch` or RAII guard. The
helper stores the original console state globally so nested activations will
restore the outer configuration correctly.

## Terminal width detection

The `cmp_readline_terminal_width()` helper returns the number of columns
available in the current terminal or `-1` when the value cannot be determined.
The Windows branch uses `GetConsoleScreenBufferInfo`, while the POSIX branch
calls `ioctl(TIOCGWINSZ)` and maps errors through the same `ft_errno` path as the
raw mode helpers.【F:Compatebility/Compatebility_readline.cpp†L45-L83】【F:Compatebility/Compatebility_readline.cpp†L119-L144】

ReadLine wraps the shim in `rl_get_terminal_width()`, which propagates the value
and ensures `ft_errno` is set to `FT_ERR_TERMINATED` when the shim reported
`-1`. This lets the higher level helpers distinguish between "not a terminal"
scenarios and valid, albeit narrow, terminals.【F:ReadLine/readline_get_terminal_width.cpp†L1-L12】

## Fallback behaviour

When the width cannot be determined, `rl_clear_line()` falls back to treating
the terminal as a one column display so prompts and input buffers are still
cleared correctly. The helper resets `ft_errno` to `FT_ERR_SUCCESSS` so callers are
not forced to special-case failures once the fallback is applied. This ensures
minimal environments—such as logs captured to files or non-interactive CI
sessions—continue to function even without full terminal capability
detection.【F:ReadLine/readline_utilities.cpp†L30-L63】

## Recommendations

- Always check the return value of `rl_get_terminal_width()` when precise layout
  is required. A value of `-1` indicates the shim could not determine the width
  and ReadLine has switched to its minimal fallback.
- Integrations that depend on advanced terminal features should perform their
  own capability probing once raw mode is enabled so they can gracefully disable
  optional UI elements when the terminal lacks the necessary support.
- Log `ft_errno` when terminal operations fail. The values come from the shared
  registry documented in `Errno/errno.hpp`, making it easier to correlate issues
  across modules.

