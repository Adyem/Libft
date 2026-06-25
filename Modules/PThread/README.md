# PThread

The `PThread` module wraps the low-level pthread-style primitives used by the rest of the library. It is the compatibility layer for mutexes, recursive mutexes, condition variables, native thread wrappers, reader/writer locks, atomic helpers, and lock tracking.

## Mutexes, Conditions, and Locks

- `pt_mutex` - Lifecycle mutex with `initialize`, `destroy`, `lock`, `unlock`, `try_lock`, ownership checks, state-lock helpers, and legacy `lockState`.
- `pt_recursive_mutex` - Lifecycle recursive mutex with the same lock/unlock/try-lock/ownership surface for recursive locking.
- `pt_condition_variable` - Condition variable with lifecycle/thread-safety helpers, `wait`, `wait_for`, `wait_until`, `signal`, and `broadcast`.
- `ft_recursive_mutex_pair_lock_order` - Describes lock order for two recursive mutexes.

## Threads and Basic PThread Wrappers

- `s_thread_id` and `pt_thread_id_type` - Native thread id wrappers.
- `AsyncData` - Async task payload used by thread helpers.
- `pt_buffer` - Public pointer/size/capacity buffer struct.
- `pthread.hpp` wrappers - Thread creation/join, sleep/yield, rwlock, and platform synchronization wrappers used by the rest of the library.

## Lock Tracking and Error Guards

- `pt_lock_tracking` - Static lock tracking interface for owned mutexes, waits, acquisitions, releases, thread exits, waiter snapshots, and per-thread state.
- `s_pt_thread_lock_info`, `s_pt_lock_wait_snapshot`, and `s_pt_lock_tracking_thread_state` - Public tracking records.

## Reader/Writer Locks

- `s_pt_rwlock` - Low-level reader/writer lock state with mutex, reader/writer conditions, active/waiting counters, strategy, and error code.
- `e_pt_rwlock_strategy` - Reader/writer lock fairness strategy enum.

Higher-level thread orchestration, cancellation, task scheduling, RAII lock helpers, and the `pt_errno_guard` compatibility wrapper now live in the separate `Threading` module. `PThread` keeps compatibility exemptions for native pthread types and legacy method names because this module is the primitive layer used by higher-level lifecycle classes.
