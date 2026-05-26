# Time

The `Time` module provides wall-clock time, monotonic time, duration structs, timezone conversion, parsing/formatting, async sleep state, benchmarking, trace events, FPS pacing, and countdown timers.

## Core Types

- `t_time` - Signed timestamp type used by the module.
- `t_monotonic_time_point` - Monotonic millisecond point with optional mutex state.
- `t_duration_milliseconds` - Duration wrapper with optional mutex state.
- `t_high_resolution_time_point` - Nanosecond-resolution time point.
- `t_time_info` - Broken-down local time fields with optional mutex state.
- `t_time_async_sleep` - Pollable async sleep state.
- `t_time_benchmark` - Online benchmark accumulator.
- `t_time_benchmark_snapshot` - Readable benchmark summary.
- `t_time_clock_now_hook` - Test hook for replacing the system clock source.

## Wall, Monotonic, and Local Time

- `time_now()` / `ft_time_ms()` / `time_now_ms()` - Return current wall time in seconds or milliseconds.
- `ft_time_format(char *buffer, ft_size_t buffer_size)` - Formats current time into a caller buffer.
- `time_monotonic()` - Returns monotonic milliseconds.
- `time_monotonic_point_now()` / `time_monotonic_point_create(...)` - Create monotonic points.
- `time_monotonic_point_add_ms(...)` / `time_monotonic_point_diff_ms(...)` / `time_monotonic_point_compare(...)` - Perform monotonic point arithmetic and comparison.
- `time_duration_ms_create(...)` - Creates a duration wrapper.
- `time_local(t_time time_value, t_time_info *out)` - Converts a timestamp to local broken-down time.
- `time_sleep(uint32_t seconds)` / `time_sleep_ms(uint32_t milliseconds)` - Sleep the current thread.

## Synchronization Helpers

- `time_monotonic_point_enable_thread_safety(...)`, `time_monotonic_point_disable_thread_safety(...)`, `time_monotonic_point_lock(...)`, `time_monotonic_point_unlock(...)`, `time_monotonic_point_is_thread_safe(...)` - Manage optional locking for monotonic points.
- `time_duration_ms_enable_thread_safety(...)`, `time_duration_ms_disable_thread_safety(...)`, `time_duration_ms_lock(...)`, `time_duration_ms_unlock(...)`, `time_duration_ms_is_thread_safe(...)` - Manage optional locking for durations.
- `time_info_enable_thread_safety(...)`, `time_info_disable_thread_safety(...)`, `time_info_lock(...)`, `time_info_unlock(...)`, `time_info_is_thread_safe(...)` - Manage optional locking for broken-down time values.

## Formatting, Parsing, and Timezones

- `time_strftime(...)` - Formats `t_time_info` with a strftime-compatible format.
- `time_format_iso8601(...)` - Allocates an ISO-8601 UTC/local timestamp string.
- `time_format_iso8601_with_offset(...)` - Allocates an ISO-8601 string with explicit offset minutes.
- `time_parse_iso8601(...)` - Parses ISO-8601 text into `std::tm` and timestamp outputs.
- `time_parse_custom(...)` - Parses text with a caller format, optionally interpreting it as UTC.
- `time_high_resolution_now(...)` - Captures a high-resolution timestamp.
- `time_high_resolution_diff_ns(...)` / `time_high_resolution_diff_seconds(...)` - Compute high-resolution elapsed time.
- `time_get_local_offset(...)` - Gets local UTC offset and daylight-saving flag.
- `time_convert_timezone(...)` - Converts a timestamp between source and target offsets.
- `time_get_monotonic_wall_anchor(...)`, `time_monotonic_to_wall_ms(...)`, `time_wall_ms_to_monotonic(...)` - Convert between monotonic and wall-clock milliseconds using an anchor.

## Async Sleep, Hooks, Benchmarks, and Tracing

- `time_async_sleep_init(...)` - Initializes a pollable sleep deadline.
- `time_async_sleep_is_complete(...)` - Reports whether the sleep has elapsed.
- `time_async_sleep_remaining_ms(...)` - Returns remaining delay.
- `time_async_sleep_poll(...)` - Polls sleep state through an event loop.
- `time_set_clock_now_hook(...)` / `time_reset_clock_now_hook()` - Install or remove the clock hook.
- `time_benchmark_init(...)` / `time_benchmark_reset(...)` - Initialize or clear benchmark accumulators.
- `time_benchmark_add_sample(...)` / `time_benchmark_add_duration(...)` - Add timing samples.
- `time_benchmark_snapshot(...)` and benchmark getters - Read sample count, average, jitter, min, max, and error state.
- `time_trace_begin_session(...)` / `time_trace_end_session()` - Start and stop trace output.
- `time_trace_begin_event(...)` / `time_trace_end_event()` / `time_trace_instant_event(...)` - Emit trace events.

## `time_fps`

- Lifecycle: `time_fps()`, `~time_fps()`, `initialize`, copy/move initialization, `destroy`, and `move`.
- `get_frames_per_second()` / `set_frames_per_second(...)` - Read or set target FPS.
- `sleep_to_next_frame()` - Sleeps until the next frame slot based on the configured FPS.
- `enable_thread_safety()` / `disable_thread_safety()` / `is_thread_safe()` - Manage optional locking.

## `time_timer`

- Lifecycle: `time_timer()`, `~time_timer()`, `initialize`, copy/move initialization, `destroy`, and `move`.
- `start(int64_t duration_ms)` - Starts a countdown.
- `update()` - Returns remaining time after updating the timer state.
- `add_time(...)` / `remove_time(...)` - Adjust remaining timer duration.
- `sleep_remaining()` - Sleeps until the timer expires.
- `enable_thread_safety()` / `disable_thread_safety()` / `is_thread_safe()` - Manage optional locking.
