# Observability

The `Observability` module records structured errors, trace events, counters, game metrics, networking metrics, and task-scheduler span metrics. Its task scheduler bridge consumes the separate `Threading` module for trace events and span timing.

## Core Observability

- `ft_observability_module` - Module id enum for file, networking, and parser counters/traces.
- `ft_observability_trace_phase` - Trace phase enum for start and finish events.
- `ft_error_context` - Structured error fields: code, platform error, module, operation, resource, and detail.
- `ft_observability_trace_event` - Trace payload containing module, phase, operation/resource, error code, and byte counts.
- `ft_observability_counters` - Aggregate operation, success, failure, read-byte, and write-byte counters.
- `ft_observability_trace_hook` - Callback invoked for trace events.
- `observability_error_context_clear(...)` - Resets an error context.
- `observability_error_context_set(...)` - Fills an error context.
- `observability_set_trace_hook(...)` / `observability_clear_trace_hook()` - Install or remove a trace hook.
- `observability_trace_emit(...)` - Emits one trace event.
- `observability_record_operation(...)` - Updates counters for a module operation.
- `observability_get_counters(...)` - Reads counters for one module.
- `observability_reset_counters(...)` - Clears counters for one module.

## Game Metrics

- `ft_game_observability_labels` - Labels for game metrics: event, entity, and attribute.
- `ft_game_observability_sample` - Game metric sample with labels, delta, total, unit, error, tag, and success flag.
- `ft_game_observability_exporter` - Callback receiving game metric samples.
- `observability_game_metrics_initialize(...)` / `observability_game_metrics_shutdown()` - Start or stop game metric export.
- `observability_game_metrics_record(...)` - Records one game metric sample.
- `observability_game_metrics_enable_thread_safety()` / `disable_thread_safety()` / `is_thread_safe()` - Manage synchronization for game metric export.

## Networking Metrics

- `ft_networking_observability_labels` - Labels for component, operation, target, and resource.
- `ft_networking_observability_sample` - Networking sample with duration, byte counts, status, error, tag, and success flag.
- `ft_networking_observability_exporter` - Callback receiving networking samples.
- `observability_networking_metrics_initialize(...)` / `observability_networking_metrics_shutdown()` - Start or stop networking metric export.
- `observability_networking_metrics_record(...)` - Records one networking metric sample.
- `observability_networking_metrics_enable_thread_safety()` / `observability_networking_metrics_disable_thread_safety()` - Manage synchronization for networking metrics.

## Task Scheduler Bridge

- `ft_otel_span_metrics` - Span timing and queue/worker metrics exported from the task scheduler.
- `ft_otel_span_exporter` - Callback receiving scheduler span metrics.
- `observability_task_scheduler_bridge_initialize(...)` - Connects scheduler tracing to an exporter.
- `observability_task_scheduler_bridge_shutdown()` - Disconnects scheduler tracing.
