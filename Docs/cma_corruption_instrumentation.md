# CMA Corruption Investigation Notes

## Temporary Diagnostics
* Added block diagnostic tracking inside `cma_free`, `cma_realloc`, and aligned allocation paths to capture the first free call site, thread, and sequence number for every block returned to CMA.
* `report_corrupted_block` and the double-free guard now emit the stored diagnostic metadata before aborting so we can identify which path released the block first.
* These diagnostics store metadata in an auxiliary list allocated with `std::malloc` so they do not rely on CMA during static initialization, but the entire tracking layer must be removed once the double-free and corruption issues in the watcher suite are resolved.

## Follow-up
* Remove the diagnostic list, helper functions, and additional logging once the allocator corruption root cause is fixed.
* Retain the `ft_thread` `start_data` allocation until `join()` or `detach()` confirms the worker has finished, preventing use-after-free writes from the helper mutex. The owner and worker now release shared references through the CMA debug hooks, and this lifetime extension must be rolled back once the investigation concludes.
* Temporarily log the allocation, thread entry, reference releases, and destruction addresses of `start_data` instances and their mutex payloads so we can correlate future corruption reports with specific thread helpers during the investigation. This logging is now implemented inside `ft_thread`, emitting `CMA DEBUG start_data` lines for allocation, thread entry, worker completion, reference releases, and final destruction events.
* Added CMA diagnostics that record the last allocation return address, thread id, and sequence number for each block so corruption reports identify where the current owner received the memory. These entries complement the existing "first free" metadata and must be removed once the investigation ends.

## Permanent fix recommendations
* Eliminate the heap-allocated `start_data` handoff by storing the launch capsule directly inside `ft_thread` (for example, via an `std::aligned_storage` buffer) and only releasing it after `join()` or `detach()` finalizes the worker. That keeps the mutex inside `ft_function` alive until the caller proves the thread can no longer reach it, removing the use-after-free window that currently lets the futex metadata overwrite CMA headers.
* Refactor `ft_function` so the worker-visible callable and its synchronization primitives live in a reference-counted holder (such as a `shared_ptr`-backed control block) shared between the creator and the worker trampoline. When both sides drop their references the holder can safely destroy the mutex and release memory back to CMA, guaranteeing that late futex writes cannot target reclaimed blocks.
* Introduce optional allocator hardening—such as padding user payloads away from headers and quarantining recently freed blocks during debug builds—to catch similar lifetime bugs earlier. A small quarantine for `start_data`-sized allocations would keep the futex metadata observable without corrupting the active free list, providing immediate signal during development while remaining disabled in production builds.
