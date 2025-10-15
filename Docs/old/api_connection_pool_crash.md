# API connection pool segmentation fault notes

## Current failure mode
- Test command: `./Test/libft_tests`
- Outcome: the suite finally runs to completion without crashing, but we forced `api_connection_pool_acquire` to flush and miss unconditionally to avoid the segmentation fault. As a result, `api connection pool reuses idle sockets for sequential requests` still fails because the reuse counter never increments, and the long-standing GNL / kv_store / YAML allocation regressions remain (1129/1143 passed overall).【bcd0e3†L1-L37】
- AddressSanitizer rebuild attempt: `COMPILE_FLAGS="-g -O1 -fsanitize=address" make -C Test re` fails during the final archive step because the sanitized module archives are emitted into `temp_objs`, so the linker never sees `API/API.a` or its peers.【bf127c†L1-L20】

## Evidence
- Because `api_connection_pool_acquire` now clears the buckets and returns early, the acquire miss counter increments every time and the idle reinsertion path never runs. The absence of reuse hits in the API regression tests confirms that pooling is effectively disabled rather than repaired.
- The sanitized rebuild never reaches the test phase due to the missing archive error, so we still lack AddressSanitizer coverage for the new code path.

## Suggested next steps
1. Reintroduce a safe pooling strategy (likely by storing value-type entries in a simple container) so `api_connection_pool_acquire` can reuse sockets again without risking the earlier segfault.
2. Update the Test build system to propagate `-fsanitize=address` through the archive creation step so sanitized rebuilds succeed, then rerun the suite to verify the short-circuit does not mask underlying memory bugs.
3. Once pooling is restored, add focused tests that assert the miss/reuse counters for sequential requests so the regression is caught without allowing the crash to return.
