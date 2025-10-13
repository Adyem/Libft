# CMA threaded hang call path notes

## Stress test entry point
- `test_cma_threaded_reallocation_stress` launches six worker threads, each running `cma_thread_mix_sequence` and later joins every worker while comparing allocation/free counters. 【F:Test/Test/test_cma_alloc.cpp†L447-L489】
- Each worker cycles through slot arrays, invoking `cma_malloc`, `cma_realloc`, and `cma_free` according to per-iteration conditions before returning a result flag to the parent test. 【F:Test/Test/test_cma_alloc.cpp†L317-L489】

## Allocator hot paths inside the workers
- `cma_thread_mix_sequence` enters `cma_malloc` for empty slots; the first operation inside `cma_malloc` constructs a `cma_allocator_guard`, which now only toggles metadata accessibility before the allocator touches shared headers. 【F:CMA/cma_malloc.cpp†L33-L41】【F:CMA/cma_utils.cpp†L39-L101】
- `cma_allocator_guard` calls `cma_lock_allocator`, which increments the metadata guard depth and ensures headers remain writable for the duration of the operation. No global mutex is involved after the thread-safety removal. 【F:CMA/cma_utils.cpp†L39-L139】【F:CMA/cma_utils.cpp†L413-L433】
- Once the guard activates, `cma_malloc` scans the free list (`find_free_block`), optionally creates a page, validates the candidate, splits it, marks it allocated, and returns the payload pointer. 【F:CMA/cma_malloc.cpp†L43-L80】

- The realloc branch in `cma_thread_mix_sequence` enters `cma_realloc`, which also creates an allocator guard, validates the header for the supplied pointer, attempts in-place growth, and otherwise allocates a new block and frees the old one while the lock remains held. 【F:CMA/cma_realloc.cpp†L14-L126】
- The free branch calls `cma_free`, which similarly constructs a guard, locates the header, validates it, merges neighbors, and frees the page before updating the global counters. 【F:CMA/cma_free.cpp†L19-L52】

## Guard unwind and metadata protection
- Every guard destructor calls `cma_unlock_allocator`, which now only decrements the metadata guard depth and restores the metadata protection mode when the last active guard exits. 【F:CMA/cma_utils.cpp†L39-L104】【F:CMA/cma_utils.cpp†L435-L444】
- If any allocator path returns before the guard is unlocked (for example, by calling `su_sigabrt` after detecting corrupted metadata), metadata protection depth remains unbalanced. The allocator keeps headers writable, but the helper threads continue running because there is no mutex to hold them in place. Investigate those branches for missing guard instances.

## Summary of guard behavior in the hot path
- `cma_lock_allocator` increments the metadata guard depth and enables read/write access to allocator headers. There is no blocking primitive in the path after the thread-safety removal. 【F:CMA/cma_utils.cpp†L413-L433】
- `cma_unlock_allocator` decrements the guard depth and restores protection when the last guard exits. If the depth underflows, the helpers report `FT_ERR_INVALID_STATE`. 【F:CMA/cma_utils.cpp†L435-L444】

These call chains show that any hang observed after the thread-safety removal stems from higher-level test logic rather than a blocked allocator mutex. Focus debugging on guard lifetimes, metadata corruption, or worker coordination issues instead of ticket-lock starvation.

## Relationship to the numbered libft tests
- In the default sorted order produced by `ft_run_registered_tests`, the CMA stress cases appear as tests 88-90. They correspond to `test_cma_randomized_stress_allocations`, `test_cma_threaded_reallocation_stress`, and the allocation-guard lifecycle checks in `test_cma_alloc.cpp`. 【F:Test/Test/test_cma_alloc.cpp†L236-L489】
- When the harness seems to pause after printing "OK 88 cma handles randomized allocation stress", it is waiting for `test_cma_threaded_reallocation_stress` to join every worker thread. With the allocator mutex removed, stalled joins point to workers that never exit their stress loop rather than threads blocked inside `cma_lock_allocator`. 【4220c0†L1-L12】【F:Test/Test/test_cma_alloc.cpp†L447-L489】

## Investigation checklist and mitigation ideas
- Capture a backtrace for each worker thread when the suite stalls. `gdb -p <pid>` or `eu-stack` will show whether they continue looping inside the stress routine or exit early because of metadata corruption. 【F:CMA/cma_utils.cpp†L39-L139】【F:Test/Test/test_cma_alloc.cpp†L317-L489】
- Add temporary logging inside `cma_allocator_guard` and the error paths that call `su_sigabrt` to verify that we always invoke `cma_unlock_allocator` before aborting. Balanced guard lifetimes keep metadata protection consistent even without the mutex. 【F:CMA/cma_utils.cpp†L39-L139】【F:CMA/cma_utils.cpp†L435-L444】
- Consider giving the stress test a watchdog: track the wall-clock duration before and after the join loop and fail the test if it exceeds a reasonable timeout instead of hanging indefinitely. That will surface the deadlock as a discrete failure in CI logs while the allocator bug is investigated. 【F:Test/Test/test_cma_alloc.cpp†L447-L489】
- Once the blocking call stack is identified, audit the corresponding allocator code paths for early returns that skip the guard destructor. In particular, ensure every branch that raises `su_sigabrt` or reports `FT_ERR_INVALID_STATE` still destroys its guard so metadata protection depth unwinds correctly. 【F:CMA/cma_malloc.cpp†L43-L80】【F:CMA/cma_realloc.cpp†L62-L132】【F:CMA/cma_free.cpp†L19-L52】
