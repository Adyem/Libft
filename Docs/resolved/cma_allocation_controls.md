# CMA allocation controls and failure injection

The CMA module exposes two toggles that control how allocations are gated and
when synchronisation occurs:

- **Allocation limit** – `cma_set_alloc_limit` defines the maximum request size
  (in bytes) the allocator will honour. A value of `0` disables the guard. The
  check runs on `cma_malloc`, `cma_realloc`, and helpers that reuse these
  routines, and it happens *before* the allocator touches shared state so the
  limit can be used to inject failures deterministically.
- **Thread-safety guard** – `cma_set_thread_safety` enables or disables the
  internal mutex that protects allocator metadata. Disable the guard only when a
  single thread interacts with the allocator, such as in focused unit tests that
  need deterministic failure injection without mutex overhead.

When thread safety is enabled, `cma_set_alloc_limit` acquires the allocator
mutex so concurrent threads cannot observe partially-written state. The helper
saves and restores the caller's `ft_errno` around the lock to avoid clobbering
unrelated error reporting. Disabling thread safety skips the lock entirely,
which keeps single-threaded tests cheap but also makes allocation-limit updates
racy if other threads touch the allocator at the same time.

## Failure-injection examples

The following snippets demonstrate how to drive the toggles during tests:

```c
/* Fail allocations that request more than 128 bytes. */
void    setup_limit_guard(void)
{
    cma_set_thread_safety(true);
    cma_set_alloc_limit(128);
    return ;
}

/* Disable the limit and clean up before leaving the test fixture. */
void    teardown_limit_guard(void)
{
    cma_set_alloc_limit(0);
    return ;
}
```

When test code must avoid mutex overhead, disable thread safety temporarily:

```c
/* Single-threaded failure-injection helper. */
void    inject_failure_without_lock(ft_size_t request_size)
{
    void    *allocation;

    cma_set_thread_safety(false);
    cma_set_alloc_limit(request_size - 1);
    ft_errno = FT_ERR_SUCCESSS;
    allocation = cma_malloc(request_size);
    /* allocation is ft_nullptr and ft_errno is FT_ERR_NO_MEMORY */
    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    return ;
}
```

Always restore both toggles once the failure scenario finishes so subsequent
code runs with the expected allocator invariants.
