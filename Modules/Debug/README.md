# Debug

The `Debug` module exposes stack trace capture/printing and address symbolization through the compatibility stack-trace helper. Crash-handler enable/disable calls are kept as portable module state toggles so the module builds on both Windows and POSIX.

## Public API

- `DBG_STACK_TRACE_MAX_FRAMES` - Maximum number of frames captured by the debug stack trace printer.
- `dbg_enable_crash_stack_traces()` - Marks crash stack traces as enabled for the module.
- `dbg_disable_crash_stack_traces()` - Marks crash stack traces as disabled for the module.
- `dbg_crash_stack_traces_enabled()` - Reports whether crash stack traces are currently enabled.
- `dbg_trace_message(...)` - Writes a plain trace line to `stderr`.
- `dbg_print_stack_trace()` - Prints the current thread's stack trace immediately.
- `dbg_symbolize_address(...)` - Resolves a code address into a symbol name and source location string.
- `dbg_get_error()` - Returns the last debug-module error code.
- `dbg_get_error_str()` - Returns readable text for the last debug-module error.
