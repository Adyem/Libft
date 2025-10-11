# Test Hang Investigation

## Observed Hang Conditions

- Several networking-oriented tests spawn a background server thread and immediately call `join()` in every early-return failure path. If the client-side setup fails before the server accepts a connection, the join blocks forever because the server thread is stuck inside a blocking `nw_accept()` call.
- `test_http_server.cpp` is representative: whenever `ft_socket` creation or `send_all` fails, the test joins `server_thread` while the thread is still waiting in `ft_http_server::run_once()`, which in turn is blocked on `nw_accept()` until a client connects.【F:Test/Test/test_http_server.cpp†L48-L155】
- The async API request test exhibits the same pattern. When `api_request_string_async` aborts early, the code joins the helper server thread even though `api_request_async_retry_server` is still blocked on `nw_accept()` and has no way to observe the failure.【F:Test/Test/test_api_request.cpp†L487-L831】

## Suggestions to Eliminate the Hang

1. **Make the server loop abortable.** Extend the helpers so the listening socket is put into non-blocking mode (e.g., pass `non_blocking = true` to `ft_http_server::start` or set `SocketConfig::_non_blocking = true`). Then, if the client setup fails, the server thread can periodically check a shared flag and exit cleanly instead of blocking forever in `nw_accept()`.
2. **Provide a failure escape hatch.** Before joining the server thread on error, actively close or unblock the listener. Examples include:
   - Trigger a dummy connection from the test failure branch so that `nw_accept()` returns and the thread can detect the failure flag.
   - Expose a shutdown helper on the server fixture that closes the listening socket; because `ft_http_server::~ft_http_server()` closes the socket, calling such a helper (or destroying the server) prior to `join()` would cause `nw_accept()` to fail and let the thread exit.
3. **Introduce join timeouts or cancellation.** Instead of an unconditional `join()`, use a cooperative flag or timed wait so that tests abort with a clear failure message rather than hanging indefinitely when the setup path cannot complete.

Implementing any of the above will prevent the suite from stalling when the client half of the test fails, and will make future debugging much easier.

## CMA Concurrency Stall Around Tests 829–830

- The `cma_allocation_guard` suite near test 829 simply wraps a freshly allocated block in an RAII guard, checks that intermediate statistics stay stable, and finally confirms the guard frees the block at scope exit.【F:Test/Test/test_cma_allocation_guard.cpp†L8-L90】【b119ad†L1-L6】  When the run pauses at this number, it is usually because the follow-up stress test (not the guard itself) has wedged.
- Test 830 (`test_cma_threaded_reallocation_stress`) launches six worker threads that repeatedly allocate, reallocate, and free slots from the shared CMA heap while verifying that byte patterns and allocator counters remain consistent before the main thread joins them.【F:Test/Test/test_cma_alloc.cpp†L317-L489】【b119ad†L7-L14】  A hang here means at least one worker never returned.

### Why the stress test can freeze

- Every allocator entry point funnels through `cma_lock_allocator`, which grabs the global ticket mutex `g_malloc_mutex`. If a code path exits without calling `cma_unlock_allocator`, or if an earlier test leaves thread safety disabled, the next worker to arrive blocks inside `pt_mutex::lock` and the join loop waits forever.【F:CMA/cma_utils.cpp†L15-L59】【F:PThread/pthread_lock_mutex.cpp†L6-L29】  Auditing those early returns and restoring `cma_set_thread_safety(true)` before kicking off the stress loop eliminates that deadlock.
- When the mutex is bypassed, the `cma_thread_mix_sequence` helper mutates shared block lists concurrently, which can corrupt the bookkeeping arrays and leave the verification loops spinning on mismatched payloads instead of finishing.【F:Test/Test/test_cma_alloc.cpp†L317-L444】  Keeping the lock engaged (or adding per-slot guards) keeps those inner loops progressing so the harness can print `OK 830`.

## 2025-02-14 Test Run

- Rebuilt the harness with `make -C Test`, which now emits the `./Test/libft_tests` executable without errors.【45344e†L1-L3】
- Executing the suite exposes early failures, notably `KO 164 ft_memcpy_s detects overlap and clears destination`, followed by multiple compression regressions (`KO 564`, `KO 570`, `KO 573`, and `KO 620`).【8c01fc†L161-L238】
- The first end-to-end run stalled after reporting `OK 829`, requiring a manual SIGINT to unblock the harness.【15c6b2†L1-L109】【6b54f5†L1-L2】
- Re-running to capture diagnostics revealed the CustomMemoryAllocator aborting with a double-free while handling `ft_file_watch` around test 1047, matching the allocator guardrail output from prior investigations.【8c01fc†L239-L312】
