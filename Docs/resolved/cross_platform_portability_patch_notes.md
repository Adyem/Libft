# Cross-Platform Portability Patch Notes

This note records the portability fixes made while building and running the test suite on the current platform.

## What Changed

- `Modules/Basic/basic_strtoul.cpp`
  - Removed an unnecessary `static_cast<uint64_t>` from the `ULONG_MAX` return path.
  - Reason: the cast triggered `-Wuseless-cast` under this toolchain and stopped the build.

- `Modules/Application/application_auth_service.cpp`
  - Switched integer formatting to the shared `FT_PRId64` macro.
  - Reason: hardcoded `"%lld"` / `"%ld"` formatting was not portable across Linux and Windows ABI conventions for `int64_t`.

- `Test/Test/test_math_average.cpp`
  - Added explicit `int64_t` casts to the large integer test arguments.
  - Reason: overload resolution for `math_average(...)` differed across platforms when the literals were left uncast.

- `Modules/System_utils/test_system_utils_runner.hpp`
  - Replaced direct `"%lld"` / `"%llu"` formatting with `FT_PRId64` / `FT_PRIu64`.
  - Reason: the test helper prints numeric values across multiple platforms and needs stable format specifiers.

- `Test/Test/test_printf_stress_allocation.cpp`
  - Updated the mixed numeric stress format string to use `FT_PRId64` / `FT_PRIu64`.
  - Reason: the stress test should exercise portable formatting paths rather than a single ABI-specific width modifier.

- `Test/Efficiency/utils.hpp`
  - Replaced `std::chrono` usage with the in-library high-resolution time helpers.
  - Reason: the efficiency harness should measure time through the library’s own `Time` module instead of introducing a separate standard-library clock path.
  - Added the shared `FT_PRId64` formatting macro for elapsed-time output.

- `Test/Test/test_networking_http_server_requests.cpp`
  - Switched the HTTP server tests from fixed ports to ephemeral ports discovered after `start()`.
  - Added a small retry wrapper around HTTP server startup in the test harness.
  - Joined the server worker before asserting its result in the GET response case.
  - Reason: the fixed test ports were vulnerable to collisions in the current environment, the GET test was reading `context.result` before the worker thread had finished, and the HTTP server startup showed a first-run flake that the retry smooths out.

- `Modules/Networking/networking_websocket_server.cpp`
  - Added exact-length socket receive handling for websocket frame headers and mask bytes.
  - Reason: stream sockets can return short reads, so parsing 2-byte headers or 4-byte masks with a single `recv()` call was flaky.

- `Modules/Networking/networking_websocket_client.cpp`
  - Added exact-length socket receive handling for websocket frame headers and mask bytes.
  - Reason: the client side had the same short-read assumption and needed the same stream-safe treatment.

- `Modules/File/file_security.cpp`
  - Removed the extra separator character from secure temp-file name generation.
  - Reason: the generated path was one character longer than the template-sized buffers used by the tests, which caused the helper to reject otherwise valid temp-file creations.

- `Modules/Logger/logger_log_rotate.cpp`
  - Kept the log sink file descriptor available after a rename failure by reopening the original file path.
  - Reason: the rename-failure test expects rotation to degrade gracefully instead of dropping the sink into an invalid descriptor state.

- `Test/Test/test_networking.cpp`
  - Switched the socket disconnect detection tests from fixed ports to ephemeral ports.
  - Reason: the hardcoded ports were still vulnerable to collisions in the current environment.

- `Modules/Networking/networking_send_utils.cpp`
  - Made `networking_check_socket_after_send()` return `1` when it observes peer disconnect via EOF.
  - Reason: the disconnect test expected a distinct disconnect signal, but the helper previously returned `0` for both the healthy and disconnected cases.

- `Test/Test/test_dumb_sound_clip_allocation_failure.cpp`
  - Passed the actual buffer size through to the secure temp-file helper.
  - Reason: `sizeof(path_buffer)` was evaluating to the pointer size in the helper, which made the temporary WAV creation fail before the allocation-limit path was exercised.

- `Test/Test/test_cma_scma_secure_wipe.cpp`
  - Looked up `Test/Full_Libft_test.a` when locating the project root and linking the runtime child binary.
  - Reason: the helper was searching for the archive in the wrong directory, so the runtime compile-and-run check exited early on the normal repo layout.

- `Docs/changelog/2025-01.md`
  - Added a short note describing the portability fixes above.
  - Reason: the project’s change log should reflect user-visible build portability adjustments.

## Why This Matters

These changes remove ABI-dependent formatting assumptions and reduce the number of platform-specific code paths in the test harness. The result is a build and test flow that behaves consistently across Linux and Windows toolchains.
