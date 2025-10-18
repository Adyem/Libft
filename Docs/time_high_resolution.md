# High-resolution timing helpers

The `Time` module now exposes a dedicated high-resolution clock that wraps
platform-specific primitives (`clock_gettime` on POSIX targets and
`QueryPerformanceCounter` on Windows). The helpers deliver nanosecond precision
and guarantee monotonic ordering so callers can safely compute elapsed
intervals across threads.

## API surface

* `bool time_high_resolution_now(t_high_resolution_time_point *time_point)`
  acquires the current tick count expressed in nanoseconds since the underlying
  platform's epoch. On success the function stores the reading in
  `time_point`, returns `true`, and clears `ft_errno`. Passing a null pointer
  sets `ft_errno` to `FT_ERR_INVALID_ARGUMENT` and returns `false`.
* `long long time_high_resolution_diff_ns(t_high_resolution_time_point start,
  t_high_resolution_time_point end)` subtracts the nanosecond counters and
  clamps overflow to `LLONG_MAX`/`LLONG_MIN` so callers can reason about
  extremely long intervals.
* `double time_high_resolution_diff_seconds(t_high_resolution_time_point start,
  t_high_resolution_time_point end)` divides the nanosecond delta by 1e9 to
  report a floating-point duration in seconds. This helper is convenient for
  logging or statistical summaries where sub-microsecond precision is not
  required.

## Precision and stability trade-offs

* On Linux and other POSIX systems the implementation prefers
  `CLOCK_MONOTONIC_RAW`, which avoids NTP adjustments and exposes the highest
  resolution available. If the kernel rejects that identifier the code
  automatically falls back to `CLOCK_MONOTONIC`, trading a small amount of
  jitter for broader compatibility.
* On Windows the helpers rely on `QueryPerformanceCounter` and normalize the
  tick frequency into nanoseconds. This API provides high precision but its
  stability depends on invariant TSC support. Systems without invariant counters
  may observe minor drift when power management features change the CPU clock
  frequency.
* Both code paths clamp the converted nanosecond reading to the `long long`
  range. Reaching that limit would require hundreds of years of uptime on
  contemporary hardware, but the guard ensures undefined behaviour does not leak
  into callers when running on specialized systems with extremely fast tick
  sources.

## Usage notes

* Prefer `time_high_resolution_now` when measuring short-lived operations such
  as profiling inner loops, encryption primitives, or rendering passes.
* The existing `time_monotonic_point_now` helper remains suitable for
  millisecond-level scheduling decisions. It is cheaper to call and pairs well
  with APIs that accept millisecond durations.
* Mixing the millisecond and nanosecond helpers is safe. Convert between them
  using the supplied difference helpers to preserve error bounds, or compute
  millisecond deltas by dividing the nanosecond result by 1,000,000 when
  appropriate.
