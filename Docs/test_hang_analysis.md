# Test Hang Investigation

## CMA stress test around case 830

### Observed symptom

Running `./Test/libft_tests` proceeds past case 830 but shortly afterwards the allocator reports metadata corruption and aborts the run. The log shows case 830 completing (`"cma handles concurrent malloc, realloc, and free operations"`) followed by subsequent CMA checks and an eventual invalid-block dump emitted from `find_free_block` just before the process aborts.【6dfdcd†L33-L41】【bcaa5c†L1-L26】  The corrupted header contains ASCII fragments from prior payloads, which indicates that user writes have spilled into the allocator bookkeeping rather than a simple double free.

### What the stress test does

`test_cma_threaded_reallocation_stress` launches six worker threads and has each execute `cma_thread_mix_sequence`. Every worker cycles through thirty-two slot buffers, alternately allocating, reallocating to a larger size, or freeing the slot while verifying that per-slot byte patterns remain intact. The helper performs 384 iterations per thread and only reports success when every slot survives the pattern checks and final cleanup without raising `ft_errno` from any allocator call.【F:Test/Test/test_cma_alloc.cpp†L317-L444】  The main test then joins all workers, checks that each reported success, and validates that the global allocation/free counters stayed balanced across the stress run.【F:Test/Test/test_cma_alloc.cpp†L447-L489】

### Why it can wedge

Every hot allocator entry point now builds on `cma_allocator_guard`, so the ticket mutex and metadata protection automatically unwind on error paths. `cma_malloc`, `cma_realloc`, and `cma_free` all instantiate the guard, validate the block, and call `unlock()` before returning, which closes the window where a worker could exit the stress loop with the CMA mutex still held.【F:CMA/cma_malloc.cpp†L47-L94】【F:CMA/cma_realloc.cpp†L142-L211】【F:CMA/cma_free.cpp†L29-L66】  The guard delegates to the ticket lock in `cma_lock_allocator`, so a stuck join at case 830 still points at either a guard that never became active or metadata that was corrupted after the guard released.【F:CMA/cma_utils.cpp†L391-L460】【F:PThread/pthread_lock_mutex.cpp†L1-L30】【F:PThread/pthread_unlock_mutex.cpp†L79-L114】

The crash dump’s overwritten sentinels implicate memory reuse rather than a missing unlock. Global `new` and `delete` route through CMA, so every `ft_thread::start_data` control block (and the `ft_function` mutex it embeds) sits on a CMA-managed allocation.【F:CMA/cma_global_overloads.cpp†L1-L104】【F:PThread/thread.hpp†L17-L90】【F:Template/function.hpp†L17-L116】  When `ft_thread::release_start_data` observes both references are gone it calls `delete`, immediately returning the block to CMA even while the worker thread still unwinds through the trampoline and futex helpers.【F:PThread/pthread_thread.cpp†L15-L174】  If the kernel finishes waking a waiter after CMA has recycled that chunk for user data, the futex write lands in the allocator headers, reproducing the ASCII fragments observed in the corrupted block log.

### Suggested fixes

- Instrument `cma_allocator_guard` to log whenever `is_active()` comes back false so the next hang points directly at the entry point that failed to take the mutex. Capturing the thread id and return address alongside that warning will confirm whether the guard itself needs additional hardening.【F:CMA/cma_utils.cpp†L391-L460】
- Keep the `ft_thread` launch capsule out of CMA until both the owner and worker confirm the trampoline has finished. Embedding the `start_data` storage inside `ft_thread`, or swapping the manual reference counting for a `shared_ptr` control block, would prevent futex wake-ups from targeting a freshly recycled CMA block.【F:PThread/thread.hpp†L17-L90】【F:PThread/pthread_thread.cpp†L15-L174】
- While investigating, add optional debug padding or a quarantine for recently freed allocations the size of `start_data`. Even a small guard region around CMA payloads would surface future scribbles immediately without remaining enabled in production builds.【F:CMA/cma_global_overloads.cpp†L1-L104】【F:Template/function.hpp†L17-L116】

These steps shift the investigation toward the cross-thread lifetime issues that still let CMA metadata get stomped after case 830, providing clearer telemetry for the next debugging pass.

## 2025-10-12 test run

- `make` completes successfully while rebuilding all 27 modules into `Full_Libft.a`.【33238f†L1-L6】
- `make -C Test` rebuilds the 202-case suite without errors.【1f4bf8†L1-L9】
- `./Test/libft_tests` finishes 1166 out of 1167 test cases before terminating with a segmentation fault.【9b5626†L1-L116】

## 2025-10-13 test run

- `make -C Test OPT_LEVEL=0 -j4` rebuilds the suite and leaves `libft_tests` ready for execution.【cad7cd†L1-L6】
- `./Test/libft_tests` stalls immediately after reporting case 829 (`cma_allocation_guard frees the managed allocation on destruction`) and never reaches the `test_cma_threaded_reallocation_stress` summary; the only additional output before the manual interrupt is a single "Allocation error" line on stderr.【b68301†L1-L47】【bf3b9b†L1-L1】

### Updated analysis

The latest stall confirms that the worker threads spawned by `test_cma_threaded_reallocation_stress` never finish joining once allocator metadata has been corrupted inside the stress routine.【7e2f14†L447-L489】  The helper threads still exchange heap control blocks through CMA, so the `ft_thread::start_data` capsule and the `ft_function` mutex it embeds continue to sit on CMA-managed storage.【F:PThread/thread.hpp†L17-L90】【F:Template/function.hpp†L17-L144】  When a worker returns from `start_routine` it immediately drops its reference and `release_start_data` can delete the capsule before the owner thread completes `join()`, reintroducing the use-after-free window noted in earlier crash dumps.【F:PThread/pthread_thread.cpp†L15-L140】  The allocator guard around every hot path still routes through the global ticket mutex, so once one worker aborts while holding that lock, the remaining threads block indefinitely on `cma_lock_allocator` and the parent test never observes their completion.【F:CMA/cma_utils.cpp†L41-L117】【F:CMA/cma_utils.cpp†L391-L466】

### Suggested follow-up

- **Extend the launch capsule lifetime.** Keep the `start_data` storage alive until `join()` or `detach()` finishes by adding a third bookkeeping reference or by embedding the capsule directly inside `ft_thread` via an aligned buffer. This prevents `release_start_data` from handing the block back to CMA while futex wake-ups and allocator bookkeeping still reference it.【F:PThread/thread.hpp†L17-L90】【F:PThread/pthread_thread.cpp†L15-L174】
- **Harden the manual reference counting.** Guard the `fetch_sub` path so that we never read `_function` fields after the final decrement, and assert that `previous_count` never underflows; add temporary logging when the last reference releases to correlate with the allocator diagnostics already captured by the corruption tracker.【F:PThread/pthread_thread.cpp†L15-L40】
- **Quarantine recent `start_data` frees in debug builds.** Teach CMA to hold on to blocks the size of `start_data` for a short grace period (or add optional padding) so any late futex writes land in the quarantine rather than overwriting live metadata, making residual bugs immediately visible without affecting production builds.【F:CMA/cma_global_overloads.cpp†L1-L104】【F:CMA/cma_utils.cpp†L451-L520】

## 2025-10-14 changes

- Replaced the manual `start_data` reference counting with a shared launch capsule owned by both the parent and worker threads so the storage is not returned to CMA until `join()` or `detach()` completes.【F:PThread/thread.hpp†L17-L82】【F:PThread/pthread_thread.cpp†L1-L116】
- Instrumented `cma_allocator_guard::is_active()` to log the thread identifier and return address when the guard fails to acquire the allocator mutex, making the next hang point directly at the offending entry point.【F:CMA/cma_internal.hpp†L36-L54】【F:CMA/cma_utils.cpp†L41-L115】

These changes address the immediate start-data lifetime hazard while providing allocator telemetry for any remaining stalls after case 829.

## 2025-10-15 test run

- `make -j4` rebuilt all 27 library modules without errors.【dbec77†L1-L15】【878b89†L1-L8】
- `make -C Test -j4` refreshed the test suite successfully.【17b82c†L1-L5】【6b678c†L1-L5】
- `./Test/libft_tests` continues to segfault; the redirected run logged progress through test 210 before the process crashed.【05ef65†L1-L86】【a3958b†L1-L2】

