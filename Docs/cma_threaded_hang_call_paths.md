# CMA threaded hang call path notes

## Stress test entry point
- `test_cma_threaded_reallocation_stress` launches six worker threads, each running `cma_thread_mix_sequence` and later joins every worker while comparing allocation/free counters. 【F:Test/Test/test_cma_alloc.cpp†L447-L489】
- Each worker cycles through slot arrays, invoking `cma_malloc`, `cma_realloc`, and `cma_free` according to per-iteration conditions before returning a result flag to the parent test. 【F:Test/Test/test_cma_alloc.cpp†L317-L489】

## Allocator hot paths inside the workers
- `cma_thread_mix_sequence` enters `cma_malloc` for empty slots; the first operation inside `cma_malloc` constructs a `cma_allocator_guard`, which attempts to lock the allocator. 【F:CMA/cma_malloc.cpp†L33-L41】【F:CMA/cma_utils.cpp†L41-L85】
- `cma_allocator_guard` calls `cma_lock_allocator`, which either toggles metadata protection when thread safety is disabled or, in the stress run, tries to acquire the global ticket mutex and increments the metadata guard depth. 【F:CMA/cma_utils.cpp†L41-L141】【F:CMA/cma_utils.cpp†L184-L220】
- `cma_lock_allocator` ultimately calls `pt_mutex::lock`, which spins on `_serving` via `pt_thread_wait_uint32` until the calling ticket becomes current. This is where hung workers block when any peer exits without dropping the allocator lock. 【F:CMA/cma_utils.cpp†L198-L218】【F:PThread/pthread_lock_mutex.cpp†L1-L29】
- Once the lock is held, `cma_malloc` scans the free list (`find_free_block`), optionally creates a page, validates the candidate, splits it, marks it allocated, and returns the payload pointer. 【F:CMA/cma_malloc.cpp†L43-L80】

- The realloc branch in `cma_thread_mix_sequence` enters `cma_realloc`, which also creates an allocator guard, validates the header for the supplied pointer, attempts in-place growth, and otherwise allocates a new block and frees the old one while the lock remains held. 【F:CMA/cma_realloc.cpp†L14-L126】
- The free branch calls `cma_free`, which similarly constructs a guard, locates the header, validates it, merges neighbors, and frees the page before updating the global counters. 【F:CMA/cma_free.cpp†L19-L52】

## Lock ownership and release
- Every guard destructor calls `cma_unlock_allocator`, propagating `lock_acquired` so only the owning thread releases the ticket mutex. 【F:CMA/cma_utils.cpp†L56-L104】【F:CMA/cma_utils.cpp†L220-L248】
- `cma_unlock_allocator` decrements the metadata guard depth and, when thread safety is enabled, calls `pt_mutex::unlock`, which resets `_owner`, advances `_serving`, and wakes the next waiter through `pt_thread_wake_one_uint32`. 【F:CMA/cma_utils.cpp†L220-L248】【F:PThread/pthread_unlock_mutex.cpp†L1-L79】
- If any allocator path returns before the guard is unlocked (for example, by calling `su_sigabrt` after detecting corrupted metadata), the corresponding worker never reaches its cleanup branch and the remaining threads stay blocked inside `pt_thread_wait_uint32`. That explains the hang observed after case 829 when a header check aborts the thread while still holding the global mutex.

## Summary of mutex use in the hot path
- **Mutex acquired:** `g_cma_allocator_mutex` inside `cma_lock_allocator` (ticket lock guarding CMA state). 【F:CMA/cma_utils.cpp†L184-L220】
- **Mutex released:** `g_cma_allocator_mutex` inside `cma_unlock_allocator` when the guard owned it. 【F:CMA/cma_utils.cpp†L220-L248】
- **Wait point:** `pt_mutex::lock`’s busy-wait loop using `pt_thread_wait_uint32`. 【F:PThread/pthread_lock_mutex.cpp†L1-L29】
- **Wake path:** `pt_mutex::unlock`, which advances the ticket and notifies one waiter. 【F:PThread/pthread_unlock_mutex.cpp†L1-L79】

These call chains show that the hang happens while worker threads wait for the single allocator mutex inside `cma_lock_allocator`. Any path that aborts or returns without unwinding the guard leaves the mutex owned, stranding the other workers in `pt_thread_wait_uint32` and preventing the parent test from observing their completion.
