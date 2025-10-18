# CMA Leak Detection

The custom memory allocator (CMA) can now track outstanding allocations on a per-thread basis to help diagnose leaks without
impacting unrelated workloads.

## Enabling leak tracking

Leak tracking is opt-in and scoped to the calling thread. Enable it before the region you want to measure, and disable or clear it
when finished:

```
cma_leak_detection_enable();
/* allocation activity */
cma_leak_detection_disable();
```

You can reset the recorded state without disabling tracking by calling `cma_leak_detection_clear()`. Clearing also resets any
previous error flag so the thread can resume tracking after an out-of-memory condition.

## Inspecting outstanding allocations

The helpers expose both quick summaries and detailed reports:

- `cma_leak_detection_outstanding_allocations()` and `cma_leak_detection_outstanding_bytes()` return counts for the current thread.
- `cma_leak_detection_report(clear_after)` emits a formatted summary that includes a per-allocation listing. Pass `true` to
  `clear_after` to clear the recorded state while keeping leak detection enabled for the thread.

The report temporarily pauses leak tracking so that formatting does not generate additional records. If clearing succeeds and
tracking was active, the helper automatically re-enables leak detection for the thread.

## Backend allocations

Backend hooks configured through `cma_set_backend` participate automatically. The leak tracker records allocations and releases for
backend pointers, including reallocation paths, so diagnostics remain consistent regardless of the active allocator backend.
