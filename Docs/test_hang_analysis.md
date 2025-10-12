# Test Hang Investigation

## CMA stress test around case 830

### Observed symptom

Running `./Test/libft_tests` proceeds past case 830 but shortly afterwards the allocator reports metadata corruption and aborts the run. The log shows case 830 completing (`"cma handles concurrent malloc, realloc, and free operations"`) followed by subsequent CMA checks and an eventual invalid-block dump emitted from `find_free_block` just before the process aborts.【6dfdcd†L33-L41】【bcaa5c†L1-L26】  The corrupted header contains ASCII fragments from prior payloads, which indicates that user writes have spilled into the allocator bookkeeping rather than a simple double free.

### What the stress test does

`test_cma_threaded_reallocation_stress` launches six worker threads and has each execute `cma_thread_mix_sequence`. Every worker cycles through thirty-two slot buffers, alternately allocating, reallocating to a larger size, or freeing the slot while verifying that per-slot byte patterns remain intact. The helper performs 384 iterations per thread and only reports success when every slot survives the pattern checks and final cleanup without raising `ft_errno` from any allocator call.【F:Test/Test/test_cma_alloc.cpp†L317-L489】  The main test then joins all workers, checks that each reported success, and validates that the global allocation/free counters stayed balanced across the stress run.【F:Test/Test/test_cma_alloc.cpp†L447-L489】

### Why it can wedge

All allocator entry points funnel through `cma_lock_allocator`, which grabs the global ticket mutex guarding CMA state whenever thread safety is enabled. A worker that reaches an early-return path without releasing the mutex leaves every other participant blocked inside the ticket lock’s wait loop, so the join in the parent test never observes that thread finishing.【F:CMA/cma_utils.cpp†L15-L22】【F:CMA/cma_utils.cpp†L184-L220】【F:PThread/pthread_lock_mutex.cpp†L1-L29】【F:PThread/pthread_unlock_mutex.cpp†L79-L114】  Even when the threads keep running, stale `ft_errno` values propagate because `cma_free` never clears the error code after a successful release; once a slot sees a previous error it tears everything down, calls `release_thread_slots`, and the main thread ends up waiting on workers that are still busy unwinding.【F:CMA/cma_free.cpp†L23-L80】

The invalid-block report in `find_free_block` confirms that the shared free-list metadata can be overwritten under this workload. Once the block sentinels are damaged, subsequent searches abort with the diagnostics seen in the log, which matches the observed stop just after the CMA stress cluster.【bcaa5c†L13-L26】  That corruption is consistent with either unsynchronized metadata writes (because a mutex was skipped) or user-level writes running past the tracked allocation size after a failed reallocation.

### Suggested fixes

- Audit every `return` path inside the allocator (especially in `cma_malloc`, `cma_realloc`, and `cma_free`) and wrap the lock/unlock pair in an RAII helper so that `cma_unlock_allocator` always runs, even on error exits. This prevents a single failure from leaving the CMA allocator mutex locked and freezing the stress test threads.【F:CMA/cma_utils.cpp†L184-L220】
- Ensure the stress harness always re-enables thread safety before launching workers and that earlier tests never leave it disabled; if any branch toggles `cma_set_thread_safety(false)` without restoring it, the next run through `cma_thread_mix_sequence` will mutate shared lists without protection and corrupt metadata exactly as observed.【F:Test/Test/test_cma_alloc.cpp†L460-L468】【F:CMA/cma_utils.cpp†L15-L22】
- Normalize `ft_errno` on successful `cma_free` (and similar helpers) so that a prior allocation failure does not cause the stress helper to misinterpret a successful free as an error and prematurely tear down its slot array.【F:CMA/cma_free.cpp†L23-L80】
- When corruption is reported, capture the offending addresses with ASLR disabled (for example via `setarch -R ./Test/libft_tests`) so that `addr2line` can map the recorded return addresses back to source locations. That shortens the feedback loop while you audit the early-return sites and size calculations involved in the failing branch.【bcaa5c†L1-L26】

These steps target both the deadlock scenario and the metadata corruption uncovered by the current run, giving the stress case a clean path to complete without hanging the suite.
