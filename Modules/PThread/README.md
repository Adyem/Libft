# PThread

The `PThread` module wraps low-level pthread-style primitives and adds higher-level concurrency helpers. It is the compatibility layer for mutexes, recursive mutexes, condition variables, threads, locks, queues, schedulers, cancellation-aware waits, and lock tracking.

## Mutexes, Conditions, and Locks

- `pt_mutex` - Lifecycle mutex with `initialize`, `destroy`, `lock`, `unlock`, `try_lock`, ownership checks, state-lock helpers, and legacy `lockState`.
- `pt_recursive_mutex` - Lifecycle recursive mutex with the same lock/unlock/try-lock/ownership surface for recursive locking.
- `pt_condition_variable` - Condition variable with lifecycle/thread-safety helpers, `wait`, `wait_for`, `wait_until`, `signal`, and `broadcast`.
- `ft_lock_guard`, `ft_recursive_lock_guard`, `ft_recursive_mutex_pair_guard`, and `ft_unique_lock` - Compatibility RAII lock utilities.
- `ft_recursive_mutex_pair_lock_order` - Describes lock order for two recursive mutexes.

## Threads and Basic PThread Wrappers

- `ft_thread` - Lifecycle thread wrapper with start/join/detach/native-id style behavior and optional thread safety.
- `s_thread_id` and `pt_thread_id_type` - Native thread id wrappers.
- `AsyncData` - Async task payload used by thread helpers.
- `pt_buffer` - Public pointer/size/capacity buffer struct.
- `pthread.hpp` wrappers - Thread creation/join, sleep/yield, rwlock, and platform synchronization wrappers used by the rest of the library.

## Concurrency Helpers

- `ft_barrier` - Lifecycle barrier with `initialize(participant_count)` and cancellation-aware `arrive_and_wait`.
- `ft_countdown_latch` - Lifecycle latch with `count_down`, `wait`, timed wait, cancellation-aware wait, and `get_count`.
- `ft_once` - Lifecycle once flag that runs a callable only once through `call` or `call_void`.
- `ft_thread_safe_queue<T>` - Lifecycle queue with `push`, `try_pop`, blocking pop, timed blocking pop, cancellation-aware pop, `empty`, and `size`.
- `ft_blocking_queue<T>` - Scheduler queue with blocking push/pop operations for task dispatch.

## Task Scheduler

- `ft_task_scheduler` - Lifecycle scheduler that manages worker threads, scheduled tasks, cancellation, timers, wait/shutdown, tracing, and optional thread safety.
- `scheduled_task` and `task_queue_entry` - Task records used by scheduler queues.
- `ft_scheduled_task_state` - Shared state for scheduled task completion/cancellation.
- `ft_scheduled_task_handle` - Handle used to cancel or inspect a scheduled task.
- `e_ft_task_trace_phase`, `ft_task_trace_event`, and task scheduler tracing callbacks - Trace scheduler submit/start/finish/cancel phases.

## Lock Tracking and Error Guards

- `pt_lock_tracking` - Static lock tracking interface for owned mutexes, waits, acquisitions, releases, thread exits, waiter snapshots, and per-thread state.
- `s_pt_thread_lock_info`, `s_pt_lock_wait_snapshot`, and `s_pt_lock_tracking_thread_state` - Public tracking records.
- `pt_errno_guard` - Compatibility guard that preserves/restores error state unless dismissed.

## Reader/Writer Locks

- `s_pt_rwlock` - Low-level reader/writer lock state with mutex, reader/writer conditions, active/waiting counters, strategy, and error code.
- `e_pt_rwlock_strategy` - Reader/writer lock fairness strategy enum.

PThread keeps several compatibility exemptions for native pthread types and legacy method names because this module is the primitive layer used by higher-level lifecycle classes.
