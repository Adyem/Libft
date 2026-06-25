# Threading

The `Threading` module groups the higher-level concurrency layer built on top of the low-level `PThread` primitives.

## Threads and Callables

- `ft_thread` - Lifecycle thread wrapper with callable construction, join/detach behavior, and optional thread safety.

## Cancellation

- `ft_cancellation_state` - Shared cancellation state with callback registration, cancellation requests, and optional thread safety.
- `ft_cancellation_source` - Owning cancellation handle used to request cancellation and create tokens.
- `ft_cancellation_token` - Lightweight non-owning token used by waits and async helpers.

## Concurrency Utilities

- `ft_once` - One-shot callable helper.
- `ft_countdown_latch` - Countdown latch with blocking and timed waits.
- `ft_barrier` - Participant barrier with cancellation-aware waits.
- `ft_thread_safe_queue<T>` - Blocking queue built on standard synchronization primitives.
- `pt_async(...)` helpers - Cancellation-aware async helpers that bridge `ft_promise` and callable wrappers.
- `ft_lock_guard`, `ft_recursive_lock_guard`, `ft_recursive_mutex_pair_guard`, and `ft_unique_lock` - RAII lock helpers used by higher-level concurrency and adapter code.
- `pt_errno_guard` - Compatibility helper that preserves and restores error state across guard pairs.

## Task Scheduling

- `ft_blocking_queue<T>` - Internal blocking queue used by the scheduler.
- `ft_thread_pool` - Lifecycle pool for submitting work, controlling worker threads, waiting/shutdown, cancellation-aware execution, error accessors, and optional thread safety.
- `ft_task_scheduler` - Task scheduler with worker threads, delayed and repeating tasks, cancellation, and metrics.
- `ft_scheduled_task_state` - Shared scheduled-task completion state.
- `ft_scheduled_task_handle` - Handle used to cancel or inspect scheduled tasks.
- `scheduled_task` and `task_queue_entry` - Internal task records used by the scheduler.
- `ft_task_trace_event` and `task_scheduler_trace_sink` - Trace event payload and callback interface.
- `task_scheduler_trace_*` helpers - Trace sink registration and span tracking utilities.

`Threading` depends on the low-level `PThread` primitives, `Template` callable/container helpers, `Time`, and the shared error/runtime layers.
