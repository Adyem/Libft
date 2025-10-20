# System health checks

The System_utils module now exposes a lightweight registration API for readiness
and liveness probes. Services can register callbacks that surface subsystem
status, then aggregate the results into structured reports for HTTP endpoints or
CLI diagnostics.

## Registering checks

```cpp
int su_health_register_check(const char *name,
                             t_su_health_check check,
                             void *context);
```

Each registered check receives its context pointer along with an output
`ft_string` it can populate with diagnostic details. The callback should return
`0` on success and set `ft_errno` plus return `-1` on failure. The helper tracks
unique names so callers can update individual checks without duplicate entries.

## Running checks

Use `su_health_run_checks` to execute every registered callback and collect
`t_su_health_check_result` entries, or `su_health_run_check` to target a
specific subsystem by name. Both helpers populate each result with the check
name, a boolean flag indicating success, the detail string supplied by the
callback, and the final error code captured from `ft_errno`.

```cpp
size_t count = 0;
t_su_health_check_result results[4];

if (su_health_run_checks(results, 4, &count) == 0)
{
    // All checks reported success.
}
else
{
    // At least one check failed and ft_errno holds the first error.
}
```

Clear existing registrations with `su_health_clear_checks` when tests need to
operate in isolation, or call `su_health_unregister_check` to remove a single
entry.

These primitives allow services to implement HTTP `/healthz` handlers, CLI
self-tests, or cron-driven smoke checks without re-implementing shared error
handling.
