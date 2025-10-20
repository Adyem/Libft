# Printf Thread-Safety Guidance

The printf subsystem offers flexible formatting helpers that frequently operate on shared buffers or global registries. This document clarifies which components are inherently thread safe, the scenarios that require external synchronization, and recommended strategies when embedding the formatters in concurrent applications.

## Overview of components

- **`pf_vsnprintf` and family.** These functions operate on caller-provided buffers and do not mutate shared state. They are reentrant as long as the destination buffer is not concurrently reused.
- **Formatter registry.** Extension points that register custom specifiers are backed by a global registry protected with an internal mutex.
- **Output adapters.** Helpers that write to `ft_string`, `ft_vector`, or logging sinks may capture references to shared containers that require their own locking.

## Thread-safety guarantees

1. Reentrant formatters (`pf_vsnprintf`, `pf_snprintf`, `pf_vsprintf`, `pf_sprintf`) are safe to call concurrently as long as each call uses distinct output buffers.
2. Internal global tables (conversion dispatchers, flag parsers) are initialized during static startup and never mutated after initialization, so they can be accessed concurrently without locking.
3. Registration helpers (`pf_register_specifier`, `pf_unregister_specifier`) acquire the internal mutex and are safe to call from multiple threads. They should be invoked during application startup to avoid contention on the hot path.
4. Helper wrappers that append directly to shared containers do **not** take locks. Callers must coordinate access to these containers.

## Recommended locking strategies

- **Per-buffer mutexes.** When multiple threads reuse the same output buffer (for example, a shared log scratch pad), guard the entire formatting-and-consume sequence with a `std::mutex` or `pthread_mutex_t`.
- **Log sink coordination.** When connecting the printf helpers to `Logger` sinks, rely on the sink's existing synchronization (e.g., the asynchronous logger's queue lock) rather than adding redundant locks.
- **Spec registration.** Wrap blocks of custom specifier registration in a single mutex if performed dynamically at runtime. Prefer static initialization where possible.
- **Formatting pipelines.** If a formatter writes into a shared `ft_string`, consider migrating to thread-local buffers that are swapped into the shared structure once the write completes under a lock.

## Testing guidance

- Add regression tests that run the same formatter concurrently across threads using the PThread test scaffolding to ensure no data races are detected by sanitizers.
- Use the existing async logger tests as references for orchestrating concurrent access patterns.

Following these guidelines keeps the printf helpers predictable in multithreaded workloads and clarifies when additional synchronization is required by the caller.
