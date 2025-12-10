# Test Run Report

- Command: `make tests`
  - Status: Succeeded.
- Command: `./Test/libft_tests`
  - Status: Aborted manually with Ctrl+C after observing CMA debug messages looping around start_data events.
  - Observed behavior: Tests progressed through at least case 829 before hanging without additional output for several minutes.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after "SocketConfig move constructor transfers values and clears source" (test #625).
  - Observed behavior: The signal handler's backtrace shows the crash occurs while `api_connection_pool_acquire` walks the connection bucket map, now failing inside `std::map::find` even after switching the pool to `std::string` keyed buckets; the lookup aborts before any pooled entry is reused.
  - Suggested follow-up: Capture the converted key string before calling `buckets.find` and compare it against the expected host/port tuple, then rerun under AddressSanitizer to catch upstream writes that might trample the key buffer.
- Command: `COMPILE_FLAGS="-g -O1 -fsanitize=address" make -C Test`
  - Status: Succeeded.
  - Observed behavior: Rebuilding the suite with AddressSanitizer succeeded after changing the bucket cache to a static local map, eliminating the potential initialization race.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault immediately after "SocketConfig move constructor transfers values and clears source" (test #625).
  - Observed behavior: AddressSanitizer reproduces the same crash path, confirming the new static-local bucket map did not resolve the corruption; stack frames still show `api_connection_pool_acquire` failing during `std::map::find`.
  - Suggested follow-up: Add bucket integrity assertions around insertions and prunes so any vector overwrite is caught before the lookup walks corrupted metadata.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after "SocketConfig move constructor transfers values and clears source" (test #625).
  - Observed behavior: Native builds still crash despite the thread-safe static bucket, so the underlying corruption likely occurs earlier in the connection lifecycle.
  - Suggested follow-up: Instrument the idle reinsertion path to dump each bucket's entry count and TLS/socket state before releasing the mutex; compare against logs from passing revisions to spot mismatches.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault near the end of the suite after "yaml_reader propagates allocation failures" (test #1055).
  - Observed behavior: The TLS registry instrumentation logs `[api_connection_pool] missing tls session=...` immediately before crashing inside `SSL_free`, confirming the dispose path is freeing a session that has already been released. Bucket dumps still show truncated keys (`plain:` or empty strings), so the underlying overwrite persists even when the double-dispose is detected.
  - Suggested follow-up: Instrument the TLS reuse path to dump the `SSL*` and `SSL_CTX*` values before reinsertion and flush buckets whose keys no longer contain the expected delimiter pattern.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault immediately after "SocketConfig move constructor transfers values and clears source" (test #625).
  - Observed behavior: The run prints `[api_connection_pool] acquire key="bad-ip:8080:plain:" buckets=1` and then crashes inside `__fprintf_chk` while dumping the bucket inventory, which indicates the cached `std::string` key for one of the entries has already been corrupted before the invalid-IP API test executes.
  - Suggested follow-up: Validate each bucket entry's key length and delimiter count before logging, and add guards in `api_connection_pool_mark_idle` to dump and flush any entry whose key mutates between insertion and reuse.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault near the end of the suite after the YAML reader tests.
  - Observed behavior: Native builds mirror the AddressSanitizer failure, crashing during `SSL_free` while draining the connection pool. Malformed bucket keys remain in the cache, indicating further validation is required before reinserting handles.
  - Suggested follow-up: Add runtime validation that counts colon separators in stored keys and flush the pool when the format is invalid, then trace the TLS acquire/idle cycle to pinpoint the overwrite.

- Command: `./Test/libft_tests`
  - Status: Completed (1129/1143 passed; no segmentation fault).
  - Observed behavior: Forcing `api_connection_pool_acquire` to clear the buckets on every call eliminates the crash, but it also disables pooling. The API reuse regression test still fails because the reuse counter stays at zero, alongside the historic GNL/kv_store/YAML failures.【bcd0e3†L1-L37】
  - Suggested follow-up: Reintroduce a safe pooling strategy that allows reuse without reintroducing the crash.

- Command: `COMPILE_FLAGS="-g -O1 -fsanitize=address" make -C Test re`
  - Status: Failed (link step could not find module archives).
  - Observed behavior: The sanitized build compiles every module but the final archive stage fails with `ar: ../API/API.a: No such file or directory`, indicating the build system expects archives in the parent directory while the sanitized invocation writes them into `temp_objs`.【bf127c†L1-L20】
  - Suggested follow-up: Adjust the Test makefile so sanitized builds place archives where the final `ar` step expects them, then rerun the AddressSanitizer suite.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault immediately after the YAML reader allocation-failure test (#1055).
  - Observed behavior: After rejecting cached handles whose TLS session failed to unregister, the pointer-based logging now shows stable key addresses and lengths for every bucket, yet the run still emits `[api_connection_pool] missing tls session=...` and aborts inside `api_connection_pool_log_state`. That indicates the TLS bookkeeping fix prevents the double free but the underlying corruption persists between the failed unregister and the subsequent logging call.
  - Suggested follow-up: Capture the handle state whenever `api_connection_pool_tls_unregister` fails (socket error code, TLS/context pointers, bucket index) and forcibly evict the entry instead of reinserting it so the remaining corruption window can be isolated to a single caller.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault immediately after "yaml_reader propagates allocation failures" (test #1055).
  - Observed behavior: With TLS pooling disabled, the suite still logs `[api_connection_pool] missing tls session=...` and then crashes inside `SSL_free` while `api_connection_pool_acquire` disposes the handle. The failing pointer never appeared in the TLS registry, indicating the owning caller freed it before the pool attempted to evict it.
  - Suggested follow-up: Instrument the TLS-backed request teardown paths to log when they bypass `api_connection_pool_mark_idle`, and capture the caller whenever `api_connection_pool_tls_unregister` fails so the premature free can be traced.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault shortly after "http2 frame encode decode roundtrip" (test #17).
  - Observed behavior: The pool state logger prints `bucket[1]` for the next API acquire attempt and then the signal handler dumps a backtrace from `api_connection_pool_acquire` before any TLS warnings fire, implying the bucket metadata is corrupt before the TLS dispose path executes.
  - Suggested follow-up: Add defensive bounds checks in `api_connection_pool_log_state` and bail out (flushing the pool) whenever the bucket size shrinks unexpectedly between reinsertion and the next acquire.

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Segmentation fault at the same location as the native run (immediately after test #17).
  - Observed behavior: AddressSanitizer mirrors the native failure without emitting heap diagnostics, which indicates the crash occurs inside the logging path before ASan detects an invalid access; the absence of `[api_connection_pool] missing tls session=...` confirms the new handshake tracking keeps the TLS registry consistent up to the point of failure.
  - Suggested follow-up: Re-run with `ASAN_OPTIONS=halt_on_error=0` and extended logging inside `api_connection_pool_track_tls_session`/`api_connection_pool_untrack_tls_session` so the sanitizer can finish reporting the offending memory access while the logs still capture the bucket contents.

- Command: `COMPILE_FLAGS="-g -O1 -fsanitize=address" make -C Test`
  - Status: Failed (linker error).
  - Observed behavior: The build finished compiling but the final link step for `libft_tests` failed with undefined references to `__asan_*` symbols because the link flags do not propagate `-fsanitize=address` to the executable.
  - Suggested follow-up: Add `-fsanitize=address` to the final link flags (e.g., via `LINK_FLAGS`) so sanitized objects from `Full_Libft.a` resolve against the ASan runtime.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault near the end of the suite after `yaml_reader propagates allocation failures` (test #1055).
  - Observed behavior: The guarded connection-pool logging shows stable bucket sizes (always size 1, capacity 1) and valid key pointers throughout the API tests, yet the crash stack trace still terminates in `SSL_free` while the YAML allocation-failure case unwinds the pooled handle.
  - Suggested follow-up: Capture the socket descriptor and TLS pointer whenever an entry is promoted or reinserted so double-free candidates can be matched against the YAML teardown path, then rerun under a debugger once the ASan link issue is resolved.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after "http2 frame encode decode roundtrip" (test #17).
  - Observed behavior: The connection pool is now hard-disabled (acquire misses unconditionally and mark-idle always evicts), yet the suite still aborts with the same stack trace inside `api_connection_pool_acquire`. No bucket logging precedes the crash because the buckets never populate, which points the investigation toward the caller teardown paths that feed handles back into the pool APIs.
  - Suggested follow-up: Instrument the async worker shutdown to record every call into `api_connection_pool_acquire`/`mark_idle`, including the handle's TLS pointer and socket descriptor, so the premature free can be traced outside of the pooling data structures.

- Command: `./Test/libft_tests`
  - Status: Failed (functional regressions persist).
  - Observed behavior: The suite now runs to completion without hitting the historical segfault, but the pool regression test still reports zero reuses and one miss, so `api connection pool reuses idle sockets for sequential requests` fails alongside the longstanding GNL, kv_store, and YAML allocation cases.【ec77d6†L1-L21】【297697†L1-L15】

- Command: `ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 ./Test/libft_tests`
  - Status: Failed (same functional regressions under AddressSanitizer).
  - Observed behavior: The sanitized build mirrors the native outcome—no crash, but the pool reuse test and other legacy failures remain, indicating the remaining issue is logical rather than a memory safety problem.【8799df†L1-L21】【a64914†L1-L15】

- Command: `make -C Test`
  - Status: Succeeded.
  - Observed behavior: Full rebuild completed without errors while updating all 27 module archives before linking the `libft_tests` binary.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after "get_next_line reports FT_ERR_NO_MEMORY when allocations fail" (test #328).
  - Observed behavior: The run progressed smoothly through the early API and CMA suites, then aborted once the GetNextLine allocation-failure regression finished; the installed signal handler dumped a backtrace rooted in `get_next_line` cleanup.
  - Suggested follow-up: Audit the leftover failure branch to ensure the combined buffer is either preserved or freed exactly once and that `ft_errno` remains set to `FT_ERR_NO_MEMORY` when returning `ft_nullptr`.

- Command: `make -C Test`
  - Status: Succeeded.
  - Observed behavior: Full rebuild completed without errors, updating all 27 module archives before linking the `libft_tests` binary.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after "get_next_line surfaces hash map allocation failures" (test #329).
  - Observed behavior: The runner's new status lines show the suite advancing past the previous leftover allocation failure before aborting; the backtrace still points into the GetNextLine leftover cleanup paths, indicating the map reset and buffer-clearing logic need additional hardening.
  - Suggested follow-up: Rebuild the leftovers cache when `ft_unordered_map` reports allocation errors and ensure `get_next_line` never accesses buffers once `leftovers` signals a failure, then re-run the focused regression to confirm the CMA free counter advances as expected.

- Command: `make -C Test`
  - Status: Succeeded.
  - Observed behavior: Full rebuild regenerated all 27 module archives before linking `libft_tests` without errors.【0b776f†L1-L32】

- Command: `./Test/libft_tests`
  - Status: Failed (1 KO, abort on test #328).
  - Observed behavior: `api_request_string_http2 falls back to http1` still reports `KO`, indicating the HTTP/2 fallback path is not setting the expected status, and the run subsequently aborts during `get_next_line surfaces hash map allocation failures` when the CMA allocator detects a double free in the leftovers cache cleanup.【5fd549†L1-L37】【8182be†L1-L12】
  - Suggested follow-up: Inspect the HTTP/2 fallback flag handling so the test sees an HTTP/1.1 retry, and harden the GetNextLine leftover failure branch to free each buffer exactly once before rebuilding the cache.

- Command: `make tests`
  - Status: Succeeded.

- Command: `./Test/libft_tests`
  - Status: Segmentation fault immediately after launch.
  - Observed behavior: The binary prints no test progress before aborting with "Segmentation fault" on stdout.

- Command: `make tests`
  - Status: Succeeded.
  - Observed behavior: Incremental rebuild refreshed the test suite artifacts without additional module work beyond regenerating the harness binaries.【89841b†L1-L2】

- Command: `./Test/libft_tests`
  - Status: Succeeded.
  - Observed behavior: The runner completed all 1,133 test cases without failures, confirming the allocator guard changes eliminated the post-suite crash and maintained CMA tracking stability.【b4f7dc†L1-L40】【7d8db8†L1-L18】
- Command: `make -C Test OPT_LEVEL=0`
  - Status: Succeeded.
  - Observed behavior: Full rebuild refreshed all 27 module archives before linking `libft_tests`, matching the standard optimized debugging build.
- Command: `./Test/libft_tests` (run 1 of 4)
  - Status: Succeeded.
  - Observed behavior: Completed all 1,173 tests without hangs after the `pt_mutex::unlock` validation adjustments, covering the async logger backlog scenario and mutex unlock recovery checks.
- Command: `./Test/libft_tests` (run 2 of 4)
  - Status: Succeeded.
  - Observed behavior: Repeated full suite passed with identical logging output, confirming the mutex regression remained resolved under a second consecutive execution.
- Command: `./Test/libft_tests` (run 3 of 4)
  - Status: Succeeded.
  - Observed behavior: Third run completed in line with prior passes; no stalls observed around the former deadlock reproduction case.
- Command: `./Test/libft_tests` (run 4 of 4)
  - Status: Succeeded.
  - Observed behavior: Final confirmation run finished the complete suite with no timeouts or deadlocks, providing confidence that the hanging behavior is no longer reproducible.
- Command: `make -C Test OPT_LEVEL=0`
  - Status: Succeeded.
  - Observed behavior: Rebuilt the suite artifacts before the extended verification campaign, refreshing all 27 module archives and relinking `libft_tests` without errors.

- Command: `./Test/libft_tests` (run 1 of 8)
  - Status: Succeeded.
  - Observed behavior: Streamed the full output to the console and completed all 1,173 tests without hangs, confirming the mutex unlock fix remains stable under a fresh run.

- Command: `./Test/libft_tests` (run 2 of 8, log captured to `/tmp/libft_run2.log`)
  - Status: Succeeded.
  - Observed behavior: Tailed log shows the YAML regression cases finishing cleanly with the suite reporting `1173/1173 tests passed`.

- Command: `./Test/libft_tests` (run 3 of 8, log captured to `/tmp/libft_run3.log`)
  - Status: Succeeded.
  - Observed behavior: Log tail again records the final YAML failure-handling tests passing followed by the overall success summary.

- Command: `./Test/libft_tests` (run 4 of 8, log captured to `/tmp/libft_run4.log`)
  - Status: Succeeded.
  - Observed behavior: Repeat execution completed without stalls; tail of the log shows the concluding YAML cases and overall pass message.

- Command: `./Test/libft_tests` (run 5 of 8, log captured to `/tmp/libft_run5.log`)
  - Status: Succeeded.
  - Observed behavior: Fifth iteration reproduced the clean YAML endings and final success report, indicating no cumulative resource leaks.

- Command: `./Test/libft_tests` (run 6 of 8, log captured to `/tmp/libft_run6.log`)
  - Status: Succeeded.
  - Observed behavior: Sixth run remained stable with the log tail showing all 1,173 tests passing without errors.

- Command: `./Test/libft_tests` (run 7 of 8, log captured to `/tmp/libft_run7.log`)
  - Status: Succeeded.
  - Observed behavior: Seventh pass maintained the streak of full-suite completions, again ending with the YAML write-failure case and the `1173/1173 tests passed` summary.

- Command: `./Test/libft_tests` (run 8 of 8, log captured to `/tmp/libft_run8.log`)
  - Status: Succeeded.
  - Observed behavior: Final verification run matched prior iterations and closed out the stress sequence without hangs or regressions.

- Command: `make -C Test`
  - Status: Succeeded.
  - Observed behavior: Incremental rebuild confirmed the suite artifacts were already current; no modules required recompilation before rerunning the tests.【84599b†L1-L5】
- Command: `./Test/libft_tests`
  - Status: Failed (1 KO, abort on test #29).
  - Observed behavior: `api_request_string_http2 falls back to http1` reported `KO`, leaving `ft_errno` at `FT_ERR_IO` because the HTTP/2 downgrade path reused a pooled plain-HTTP connection that the server had already closed, so the fallback saw a null body instead of `"Hello"`.【3aa9da†L1-L33】【e315e5†L1-L2】
  - Potential fixes to investigate:
    - Add a liveness probe when reacquiring pooled sockets (for example `recv(..., MSG_PEEK)` or a zero-timeout poll) so entries where the peer has already closed the connection are discarded rather than handed to the HTTP/2 path.
    - When `api_http_execute_plain_http2` bails out before writing any bytes, treat the handle as tainted: close the socket, skip pooling, and let the plain HTTP fallback establish a fresh connection instead of reusing the potentially severed one.
    - On successful HTTP/1 fallbacks, mark the connection handle to avoid being reinserted into the pool unless a response body was actually read, keeping the cache from filling with sockets that were closed mid-flight by the origin.
- Command: `valgrind --tool=memcheck --track-origins=yes ./Test/libft_tests`
  - Status: Aborted manually after interrupting the hang during test #30.
  - Observed behavior: Valgrind flags repeated "Invalid read of size 8" reports inside `cma_realloc` while the HTTP client builds request strings and buffers downgrade responses, pointing to CMA metadata access while `ft_string::append` and `ft_string::operator+=` grow their backing allocations.【2cb46c†L1-L6】【cd7b4c†L1-L17】【400ec0†L1-L80】【f8a5e5†L1-L14】 The suite still fails test #29 with a null downgrade body and then stalls on the retry case, matching the native run but now tied to allocator guard activity in the downgrade helpers.【f8a5e5†L1-L14】【25c8cc†L1-L25】
  - Suggested follow-up: Avoid funneling downgrade bytes through `ft_string` so the fallback path no longer depends on CMA reallocation. For example, once `api_http_execute_plain_http2_once` confirms the HTTP/1 headers, allocate the final body buffer directly with `cma_malloc` and copy from `handshake_buffer`/chunk decoder into that block instead of pushing through `decoded_body`/`ft_string`. That keeps the downgrade path from exercising the `cma_realloc` guard and should eliminate the null-body regression without reopening the stale-socket issue.
- Command: `valgrind --tool=memcheck --track-origins=yes ./Test/libft_tests` (run after guarding `cma_realloc` returns)
  - Status: Aborted manually after the known KO at test #29 and the hang at test #30.
  - Observed behavior: The HTTP/2 downgrade test still reports `KO 29`, and the retry scenario continues to block, but Valgrind no longer emits any "Invalid read" diagnostics—`memcheck` exits with `ERROR SUMMARY: 0 errors from 0 contexts`, confirming that capturing the user pointer before releasing the CMA metadata guard eliminated the prior allocator faults.【4be742†L1-L16】【127087†L1-L1】【3b2baf†L1-L24】
  - Suggested follow-up: With the allocator instrumentation quiet, refocus on the HTTP downgrade logic (for example, why the plain HTTP fallback still returns a null body) without the distraction of CMA guard violations.
- Command: `valgrind --tool=memcheck --track-origins=yes ./Test/libft_tests` (current run)
  - Status: Aborted manually after the downgrade KO and retry hang.
  - Observed behavior: Tests progressed cleanly through case 28 before `api_request_string_http2 falls back to http1` still reported `KO`, leaving the runner stuck on the retry scenario until it was interrupted; Memcheck’s summary remained at `0 errors from 0 contexts`, so the CMA guard fixes continue to hold even though the null-body regression persists.【4412e9†L1-L16】【a3bb52†L1-L1】【5ae686†L1-L2】【214beb†L1-L24】
  - Additional diagnostics: A matching debugger run shows the retry path raising `SIGPIPE` as soon as it attempts to resend over the downgraded socket, confirming the client is still writing to a descriptor the origin has already closed.【23128e†L1-L9】
  - Suggested follow-up: When the downgrade handshake already buffered a complete HTTP/1 response, finalize that buffer immediately, flag the handle as closed, and skip issuing another plain-HTTP request on the same socket; if more data is required, evict the entry and establish a fresh connection before retrying so the fallback no longer writes to a dead descriptor.

- Command: `./Test/libft_tests` (instrumented run with fallback logging)
  - Status: Failed (KO at test 29, hang at test 30).
  - Observed behavior: The new instrumentation shows the plain-HTTP fallback returns `FT_ERR_IO` (`error_code=12`) even though `ft_errno` has already been reset to `FT_ER_SUCCESSS`, so callers still see a null body without an error code while the suite wedges waiting on the retry scenario.【e58ef4†L1-L1】【7932a5†L1-L3】
  - Suggested follow-up: Propagate the fallback’s `FT_ERR_IO` status back through `api_request_string_http2` (instead of zeroing the error code before returning) so the regression test fails with a meaningful errno and the retry harness can bail out cleanly.
