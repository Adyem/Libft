# CMA Metadata Protection Segfault Analysis

## Observed Failure
- The test suite segfaults shortly after case 1164 when CMA metadata pages are flipped back to `PROT_NONE`. The allocator logs show the crash occurs directly after `ft_thread::release_start_data` deletes a launch capsule that was allocated from CMA, leaving `_start_data` dangling until the main thread later calls `join()`.

## Suggested Fixes
1. **Keep the launch capsule alive until `join`/`detach` completes.**
    - Instead of deleting the `start_data` structure as soon as the worker thread releases its reference, defer reclamation until the owning `ft_thread` releases its pointer. Embedding the capsule directly inside `ft_thread` or wrapping it in a reference-counted smart pointer would prevent the owner from touching freed memory during `join()`/`detach()`.
2. **Guard manual reference counting in `ft_thread::release_start_data`.**
    - Move the `fetch_sub` test ahead of the debug logging so we do not read `data->function` after the final release decrements the reference count to zero. Alternatively, add a `fetch_add` in the constructor to start at three references (owner, worker, bookkeeping) and only destroy the capsule when the bookkeeping token is cleared after `join()`.
3. **Audit late accesses to `_start_data` after release.**
    - `ft_thread::join()` and `ft_thread::detach()` both call `release_start_data(this->_start_data)` even when `_start_data` may already have been cleared on earlier error paths. Add a guard that sets `_start_data` to `ft_nullptr` before issuing the final release, ensuring we do not accidentally reuse the stale pointer elsewhere.
4. **Add a scoped metadata guard helper.**
    - Replace the open-coded `cma_lock_allocator`/`cma_unlock_allocator` pairs with an RAII wrapper that automatically bumps `g_cma_metadata_access_depth`. This will guarantee every metadata touch, including nested logging, happens while headers are writable, preventing similar crashes when new call sites are added.

These changes focus on eliminating the `ft_thread` use-after-free that the new metadata protections now expose while keeping the allocator protections intact.
