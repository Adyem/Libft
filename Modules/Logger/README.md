# Logger

The `Logger` module provides global and object-oriented logging with levels, file rotation, structured fields, sinks, syslog/remote sinks, redaction, async queues, remote health checks, context stacks, and lock-contention sampling.
Formatted messages are also recorded into the separate `Sink` module so log history can be inspected without depending on the full logger stack.

## Types

- `t_log_level` - Log severity enum: debug, info, warn, error, none.
- `t_log_sink` - Sink callback receiving formatted messages and user data.
- `s_log_remote_health` - Remote sink health status.
- `s_log_field` - Structured logging key/value field with optional synchronization helpers.
- `s_log_async_metrics` - Async queue pending/peak/dropped counters with optional synchronization helpers.
- `s_log_lock_contention_sample` - One sampled mutex wait.
- `s_log_lock_contention_statistics` - Aggregate contention statistics.
- `ft_log_context_guard` - Lifecycle guard that pushes structured context fields and pops them on destroy unless released.
- `ft_logger` - Lifecycle logger object that can become the global logger.

## Global Configuration

- `ft_log_set_level(...)` - Sets global minimum log level.
- `ft_log_set_file(...)` - Logs to a file with a maximum size.
- `ft_log_set_rotation(...)` / `ft_log_get_rotation(...)` - Configure or read log rotation limits.
- `ft_log_close()` - Closes configured log output.
- `ft_log_set_alloc_logging(...)` / `ft_log_get_alloc_logging()` - Toggle allocator logging; the underlying allocator flag is mirrored through `CMA` and its messages are recorded into `Sink`.
- `ft_log_set_api_logging(...)` / `ft_log_get_api_logging()` - Toggle API logging.
- `ft_log_set_color(...)` / `ft_log_get_color()` - Toggle colored terminal output.
- `ft_log_add_sink(...)` / `ft_log_remove_sink(...)` - Manage custom sinks.
- `ft_json_sink(...)` / `ft_syslog_sink(...)` - Built-in structured sink callbacks.
- `ft_log_set_syslog(...)` / `ft_log_set_remote_sink(...)` - Configure syslog or remote logging.
- `ft_log_add_redaction(...)`, `ft_log_add_redaction_with_replacement(...)`, and `ft_log_clear_redactions()` - Manage redaction rules.

## Logging, Context, Async, and Contention

- `ft_log_debug`, `ft_log_info`, `ft_log_warn`, `ft_log_error` - Format and emit messages at the matching level.
- `ft_log_structured(...)` and level-specific structured variants - Emit structured messages with field arrays.
- `ft_log_context_push(...)`, `ft_log_context_pop(...)`, and `ft_log_context_clear()` - Manage thread/global context fields.
- `ft_log_enable_async(...)`, `ft_log_enqueue(...)`, queue-limit getters/setters, and async metrics helpers - Manage asynchronous logging.
- `ft_log_enable_remote_health(...)`, `ft_log_set_remote_health_interval(...)`, `ft_log_probe_remote_health()`, and `ft_log_get_remote_health(...)` - Manage remote sink health checks.
- `ft_log_enable_lock_contention_sampling(...)`, interval/threshold setters, sample retrieval, statistics retrieval, and statistics reset - Manage lock-contention telemetry.
- `log_field_*` and `log_async_metrics_*` helpers - Prepare, tear down, lock, and unlock public metric/field structs.

## `ft_log_context_guard`

- Lifecycle: `initialize`, field-array initialization, copy/move initialization, `destroy`, and `move`.
- `release()` - Leaves pushed context active after the guard is destroyed.
- `is_active()` - Reports whether the guard still owns pushed context.
- `get_error()` / `get_error_str()` - Return guard error state.

## `ft_logger`

- Lifecycle: constructor, destructor, `initialize`, configured initialization, copy/move initialization, `destroy`, and `move`.
- Configuration methods mirror the global API: level, file, rotation, sinks, allocation/API/color flags, syslog, remote sink, async queue, remote health, and thread safety.
- Context methods: `push_context`, `pop_context`, and `make_context_guard`.
- Emission methods: `debug`, `info`, `warn`, `error`, `structured`, and level-specific structured variants.
- Error methods: `get_error()` and `get_error_str()`.
## Additional API

- `ft_log_context_push_field(const char *key, const char *value)` - Adds one valued field to the current logging context.
