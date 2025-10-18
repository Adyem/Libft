# System Utilities Resource Lifetime Tracing

The System utilities module now exposes a lightweight tracing facility that allows
callers to register callbacks invoked whenever `su_abort` or `su_assert` trigger.
This makes it possible to capture diagnostics about outstanding resources before
process termination.

## Registering tracers

Use `su_register_resource_tracer` to add a callback that receives the abort
reason string. The reason is the message supplied to `su_assert`, or
"su_abort invoked" when the process exits through `su_abort` directly.

```cpp
static void log_inflight_transactions(const char *reason)
{
    my_transaction_registry_dump(reason);
    return ;
}

su_register_resource_tracer(&log_inflight_transactions);
```

If the tracer should no longer run, call `su_unregister_resource_tracer`. Both
registration and removal translate allocation errors or missing entries into
`ft_errno` for easy automation.

## Manual execution

`su_run_resource_tracers` may be invoked directly to emit diagnostics during
controlled shutdown paths. The helper replays the registered tracers without
terminating the process, allowing tests to assert on tracer behaviour.

## Cleaning up

`su_clear_resource_tracers` removes every registered callback. This is primarily
intended for test fixtures so suites can reset the tracer registry between test
cases without restarting the process.
