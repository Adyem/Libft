# Debug

The `Debug` module exposes crash-time stack trace support, direct stack trace printing, and address symbolization through the compatibility stack-trace helper.

## Public API

- `DBG_STACK_TRACE_MAX_FRAMES` - Maximum number of frames captured by the debug stack trace printer.
- `dbg_enable_crash_stack_traces()` - Installs crash handlers that print a stack trace before process termination.
- `dbg_disable_crash_stack_traces()` - Removes the crash stack trace handlers installed by this module.
- `dbg_crash_stack_traces_enabled()` - Reports whether crash stack trace handlers are currently enabled.
- `dbg_print_stack_trace()` - Prints the current thread's stack trace immediately.
- `dbg_symbolize_address(...)` - Resolves a code address into a symbol name and source location string.
- `dbg_get_error()` - Returns the last debug-module error code.
- `dbg_get_error_str()` - Returns readable text for the last debug-module error.
