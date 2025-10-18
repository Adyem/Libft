# System utilities environment sandboxing

The System utilities module now exposes helpers that snapshot and restore the
process environment. Tests and automation can rely on these APIs to ensure that
mutations performed during a scenario do not leak into subsequent runs.

## API overview

| Helper | Description |
| --- | --- |
| `su_environment_snapshot_capture` | Copies every `NAME=VALUE` pair from the current process environment into the provided snapshot. |
| `su_environment_snapshot_restore` | Restores the environment to match the captured snapshot by removing variables that were absent and repopulating the preserved values. |
| `su_environment_snapshot_dispose` | Clears the snapshot contents once the caller no longer needs the captured state. |
| `su_environment_sandbox_begin` | Convenience wrapper around `su_environment_snapshot_capture` that prepares a snapshot for a sandboxed test section. |
| `su_environment_sandbox_end` | Restores the captured environment and clears the snapshot in a single call. |

All helpers return `0` on success and `-1` on failure. They set `ft_errno` to one
of the documented error codes when a problem occurs. Invalid arguments produce
`FT_ERR_INVALID_ARGUMENT`, allocation failures surface `FT_ERR_NO_MEMORY`, and
environment mutation errors reuse the values propagated by `ft_setenv` and
`ft_unsetenv`.

## Usage patterns

### Manual snapshot and restore

```c++
t_su_environment_snapshot snapshot;
if (su_environment_snapshot_capture(&snapshot) != 0)
    return ;
/* mutate the environment */
su_environment_snapshot_restore(&snapshot);
su_environment_snapshot_dispose(&snapshot);
```

### Sandbox guard

```c++
t_su_environment_snapshot sandbox;
if (su_environment_sandbox_begin(&sandbox) != 0)
    return ;
/* execute code that modifies the environment */
su_environment_sandbox_end(&sandbox);
```

The sandbox helpers ensure that the environment returns to its original state
even when tests add or remove variables. This keeps suites hermetic and prevents
flaky interactions between unrelated cases.
