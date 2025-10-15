# TODO

## Library-wide priorities
- [x] Publish a canonical error-code registry that documents each value exposed through `Errno/errno.hpp` and clarifies which modules set them, so cross-module error handling stays consistent.
- [x] Re-audit the error code catalog to collapse redundant class-specific values into shared generic codes where possible so callers can rely on a simpler set of outcomes when handling failures.
- [x] Deduplicate and auto-generate `FullLibft.hpp` so umbrella includes stay sorted and avoid the current duplicate entries like `System_utils/system_utils.hpp` (manifest-driven generator keeps the header updated).

## Documentation and examples
- [x] Produce per-module overviews that explain design goals, invariants, and error-reporting patterns referenced throughout `README.md` (see `Docs/module_overviews.md`).
- [x] Generate Doxygen (or Sphinx/Breathe) API references so consumers can navigate the large header surface (see `Docs/Doxyfile` and `tools/run_doxygen.py`).
- [x] Add copyable examples for common tasks (file I/O, HTTP client usage, task scheduling) showing how `_error_code` flows should be checked.
- [x] Document platform support expectations and how the `Compatebility` shims should be extended when adding new targets.
- [x] Maintain an FAQ and troubleshooting guide informed by common issues raised in bug reports or support channels.
- [x] Offer quick-start templates (CMake, Meson, Bazel) demonstrating how to consume the library in fresh projects.
- [x] Publish architecture decision records (ADRs) for major subsystem changes to capture context and trade-offs. See `Docs/architecture_decision_records/`.

## Module-specific improvements

### Libft (core C utilities)
- [x] Implement missing staples such as `ft_split`, `ft_strdup`, `ft_strjoin`, and `ft_itoa` to complete the libc-like surface offered in `libft.hpp` (already exposed today through the CMA wrappers alongside the Libft exports).
- [x] Add bounded counterparts so callers can avoid manual truncation checks.
  - [x] Provide `ft_strnlen` and `cma_strndup` implementations that mirror libc semantics while propagating `ft_errno` on failure.
  - [x] Provide span-friendly adapters that operate on buffers without null terminators (`ft_span_dup`, `ft_span_to_string`).
  - [x] Add `ft_strncpy_s` and `ft_strncat_s` helpers that fail fast on truncation and zero buffers when bounds are exceeded.
- [x] Document and reconcile the overlap between CMA-provided string helpers and the Libft implementations so their semantics and error codes do not drift apart (README now maps shared error codes and regression tests cover the memdup overlap).
- [x] Extend `ft_to_string` to avoid `std::ostringstream` overhead, propagate `ft_string` allocation failures, and offer formatting for unsigned and floating-point types.
- [x] Provide wide-character helpers (`ft_wstrlen`, UTF-16/32 conversion) to complement the existing UTF-8 routines.
- [x] Harden environment helpers (`ft_setenv`, `ft_getenv`) with thread-safety guards and explicit documentation about process-wide side effects.
- [x] Add optional bounds-checked wrappers for memory and string functions that return rich error codes instead of sentinel values.
- [x] Introduce compile-time configuration flags that allow trimming unused helpers for embedded builds.
- [x] Add locale-aware collation and case-folding helpers that rely on the `System_utils` abstractions when available.

### CMA (custom memory allocation)
- [x] Document how the allocation limit and thread-safety toggles interact, including examples that demonstrate failure injection. (See `Docs/cma_allocation_controls.md` and new regression tests.)
- [x] Add stress tests that mix `cma_malloc`, `cma_realloc`, and `cma_free` across threads to validate the internal bookkeeping under contention.
- [x] Surface allocator statistics (total allocated bytes, peak usage) through a public query API for diagnostics.
- [x] Provide hooks for custom backends so projects can route allocations to region-specific arenas or embedded allocators.
- [x] Audit the global new/delete overrides to ensure they respect alignment requirements on every supported platform.
- [ ] Implement guard-page or canary instrumentation in debug mode to catch buffer overruns during development.
- [ ] Add leak detection and reporting helpers that can be toggled per-thread for targeted investigations.
- [x] Supply RAII helpers and scope guards that automatically free CMA allocations to simplify error paths in callers.

### Compression
- [x] Allow callers to configure input/output buffer sizes in `ft_compress_stream` and `ft_decompress_stream` instead of hard-coding 4096-byte stacks.
- [x] Differentiate zlib error mappings so `map_zlib_error` returns distinct codes for buffer exhaustion versus malformed input.
- [x] Offer in-memory convenience helpers that compress or decompress between `ft_string`/`ft_vector` instances without touching file descriptors.
- [x] Expand streaming tests to cover flush behavior, incremental hook replacement via `compression_stream_test_hooks.hpp`, and truncated inputs.
- [ ] Evaluate adding alternative algorithms (e.g., LZ4, Brotli) behind a common interface with capability detection.
- [ ] Provide progress callbacks and cancellation support for long-running compression tasks.
- [ ] Document tuning guidelines (window sizes, compression levels) and expose presets optimized for speed vs. ratio.

### Logger
- [x] Provide structured logging helpers (JSON key/value or printf-style macros) so log consumers avoid manual string assembly.
- [x] Add backpressure metrics and drop counters for the asynchronous logger to expose when the worker thread cannot keep up.
- [x] Implement log rotation policies based on age and size, including retention limits and unit tests that verify rollover correctness.
- [ ] Expose sink health probes that periodically ping remote targets configured via `ft_log_set_remote_sink` and surface connection errors early.
- [ ] Document how `_alloc_logging` and `_api_logging` flags influence emitted messages to prevent confusion when toggling modes.
- [ ] Add redaction helpers for sensitive fields and ensure they integrate with formatting utilities.
- [ ] Support structured context propagation (per-thread request IDs, correlation tokens) through scoped guards.
- [ ] Restore the `logger async logging` regression after fixing the heap corruption triggered by asynchronous sink teardown.

### Networking
- [ ] Complete HTTP/2 support by honoring SETTINGS frames, stream prioritization, and flow-control windows in `http2_client`.
- [ ] Integrate ALPN negotiation into the higher-level API wrappers so HTTP/2 is automatically selected when the server supports it.
- [ ] Build a reusable DNS resolver (with caching and IPv6 support) that the socket helpers can share instead of assuming raw host strings.
- [ ] Add connection pooling and keep-alive management for `http_client`/`http_server` to reduce socket churn under load.
- [ ] Harden WebSocket handling with per-message deflate support and stricter frame validation (masking, RSV bits, control frame sizes).
- [ ] Provide non-blocking, event-loop-friendly wrappers for UDP sockets that integrate with epoll/kqueue abstractions already present.
- [ ] Introduce QUIC/HTTP3 experimentation behind feature flags, reusing TLS primitives where possible.
- [ ] Add observability hooks (latency histograms, error tagging) that integrate with Prometheus or OpenTelemetry exporters.
- [ ] Provide RAII socket handles that close file descriptors automatically and coordinate Windows `WSAStartup` lifecycle management.
- [x] Fix the Winsock error handling path in `ft_socket::setup_server` so the helper uses `WSAGetLastError()`/`ft_errno` instead of `errno`, otherwise Windows failures in `create_socket`, `set_reuse_address`, timeouts, bind, or listen propagate the wrong code. 【F:Networking/networking_setup_server.cpp†L18-L146】【F:Networking/networking_setup_server.cpp†L188-L237】
- [x] Update the HTTP server response writer to translate `nw_send` failures via `ft_errno`/`WSAGetLastError()` instead of raw `errno`, which currently reports success on Windows because Winsock does not set `errno`. 【F:Networking/http_server.cpp†L1-L14】【F:Networking/http_server.cpp†L278-L307】
- [x] Make `ft_socket::initialize` return an error when `setup_server`/`setup_client` fails so callers are not forced to poll `_error_code` after a `0` return. 【F:Networking/networking_socket_class.cpp†L395-L423】
- [ ] Restore the `http2 stream manager tracks streams` regression once the stream manager no longer crashes when tracking concurrent streams.
- [ ] Reintroduce the HTTP/2 header compression roundtrip regression once `http2_compress_headers`/`http2_decompress_headers` stop aborting the suite (current implementation trips an abort while the test runner executes "http2 header compression roundtrip").
- [ ] Restore the `websocket client detects invalid handshake` regression after fixing the crash in the websocket client handshake teardown path.
- [ ] Restore the `HTTP server echoes POST body` regression once the HTTP server request pipeline stops corrupting heap allocations during body echoing.

### API (HTTP client facade)
- [ ] Support HTTP/2 upgrades and streaming request/response bodies within `api_request` so large transfers do not buffer entirely in memory.
- [ ] Add retry, backoff, and circuit-breaker policies that map transport failures into actionable error codes.
- [ ] Surface detailed TLS handshake diagnostics (certificate chain, cipher selection) through `api_tls_client` for observability.
- [ ] Provide mockable interfaces or dependency injection hooks to simplify unit testing of API consumers.
- [ ] Offer request signing helpers (HMAC, OAuth) for authenticated services, leveraging the Encryption module.
- [ ] Emit structured metrics per endpoint (latency, throughput, error rate) suitable for dashboards.
- [ ] Restore the `api_request_string_http2 falls back to http1` regression once the fallback path properly toggles the HTTP/2 flag and returns the downgraded response body.
- [ ] Restore the `api_request_string_host_bearer adds bearer authorization header` regression after fixing the heap corruption triggered by the bearer-token request path.
- [ ] Restore the `api_request_string_host_basic adds basic authorization header after existing headers` regression once the header assembly stops corrupting heap allocations.

### PThread and concurrency utilities
- [ ] Extend `task_scheduler` with work-stealing queues and task affinity controls to better utilize multi-core systems.
- [ ] Implement timed wait variants for mutexes and condition variables mirroring `std::timed_mutex` semantics.
- [ ] Add reader-priority and writer-priority strategies to the RW lock implementation with accompanying benchmarks.
- [ ] Offer cooperative cancellation tokens that integrate with `thread_pool` in `Template/` to unify async control flows.
- [ ] Provide structured tracing of task lifecycles to aid debugging of scheduling stalls.
- [ ] Add lock contention diagnostics (sampling, priority inversion detection) exposed through the Logger module.

### Template containers and utilities
- [x] Review container naming (e.g., rename `unordened_map.hpp` to `unordered_map.hpp`) and ensure API parity with the standard library counterparts.
- [x] Add iterator invalidation rules and complexity annotations to headers so users know when operations are safe in real-time systems.
- [ ] Provide small-buffer optimizations or node pooling options for frequently-used containers (`vector`, `map`, `trie`).
- [ ] Expand constexpr support where possible so templates can be used in compile-time contexts.
- [ ] Write exhaustive tests for allocator-aware paths, ensuring `_error_code` mirrors allocation failures.
- [ ] Introduce serialization helpers that interoperate with JSON/YAML encoders for easy persistence of container contents.
- [ ] Add concept-constrained overloads in C++20 builds to improve diagnostic quality when templates are misused.

### GetNextLine
- [ ] Design and implement a custom STREAM abstraction that the streaming helpers can rely on instead of the standard library facilities.
- [ ] Reintroduce the streaming helpers (`ft_read_file_lines`, `ft_open_and_read_file`) once the leftovers cache stabilizes under allocation failures and the custom STREAM implementation is available.
- [ ] Restore the leftover allocation failure regression test and fix the get_next_line cleanup so buffers are released exactly once when reinserting cached data fails.
- [ ] Restore the `get_next_line surfaces hash map allocation failures` regression after the leftovers cache can safely recover from map allocation errors without leaking entries.

### CPP_class (utility classes)
- [ ] Add move-aware and noexcept constructors/destructors to complex classes like `ft_big_number` and `DataBuffer` to improve performance guarantees.
- [ ] Ensure all classes expose serialization helpers compatible with the Storage and Networking modules.
- [ ] Provide unit tests that exercise error propagation on `_error_code` members across copy/move operations.
- [ ] Document ownership semantics for wrappers like `ft_file` to avoid double-close mistakes.
- [ ] Audit exception safety for each class and document the strong/weak guarantees they provide.
- [ ] Supply sample adapters showing how to embed these classes into user-defined types with RAII expectations.

### Storage
- [ ] Optimize TTL pruning in `kv_store` to avoid copying keys into a `std::vector` during every sweep.
- [ ] Introduce transactional batch operations (multi-set, compare-and-swap) so clients can update related keys atomically.
- [ ] Add persistence backends (JSON lines, SQLite, memory-mapped files) selectable at runtime.
- [ ] Create background compaction or vacuum routines that reclaim expired data without blocking callers.
- [ ] Implement snapshot/export tooling for backups and debugging.
- [ ] Provide replication hooks (write-ahead log shipping, follower sync) for high-availability deployments.
- [ ] Surface observability metrics (hit ratio, compaction duration) for integration with dashboards.

### JSon
- [ ] Restore the `json reader reports io errors` regression once file-based reads surface missing files without corrupting allocator state.

### Time utilities
- [ ] Provide high-resolution timers that wrap `clock_gettime`/`QueryPerformanceCounter` and document precision trade-offs.
- [ ] Add timezone conversion helpers, daylight-saving awareness, and formatting options for ISO-8601 with offsets.
- [ ] Implement async-friendly sleep primitives that integrate with event loops instead of blocking threads.
- [ ] Supply benchmarking helpers that record rolling averages and jitter for repeated measurements.
- [ ] Offer monotonic-to-wall-clock translation utilities to reconcile timestamps across modules.
- [ ] Integrate profiling markers compatible with Chrome tracing or perfetto for visualizing timelines.

### RNG
- [ ] Add cryptographically secure RNG wrappers that rely on system entropy sources with clear fallbacks.
- [ ] Provide deterministic PRNG seeds and stream splitting utilities for reproducible simulations.
- [ ] Expand distribution coverage (gamma, beta, chi-squared) and ensure PDF/CDF helpers are available.
- [ ] Document statistical quality guarantees and include tests that run chi-squared or KS checks.
- [ ] Supply vectorized sampling routines that take advantage of SIMD when available.
- [ ] Publish guidance on combining RNG streams safely to avoid correlation in parallel workloads.

### Math
- [ ] Complete the linear algebra module with vector, matrix, and quaternion operations optimized using SIMD when available.
- [ ] Introduce polynomial solvers, interpolation (Bezier, spline), and numerical integration helpers.
- [ ] Provide interval arithmetic and error-propagation utilities for robust scientific calculations.
- [ ] Add unit tests that validate tolerance-based comparisons and edge-case handling (NaN, infinities).
- [ ] Implement automatic differentiation primitives for optimization and machine-learning use cases.
- [ ] Supply FFT and convolution helpers that reuse existing complex-number utilities.

### Encryption
- [ ] Expand beyond SHA-1 by adding SHA-2/3, BLAKE2, and streaming HMAC helpers.
- [ ] Introduce authenticated encryption (AEAD) wrappers that integrate with the Networking module's TLS stack.
- [ ] Document key management best practices and provide secure random key generation utilities.
- [ ] Add self-tests that verify algorithm outputs against known test vectors.
- [ ] Provide hardware acceleration hooks (AES-NI, ARMv8 crypto) with graceful fallback paths.
- [ ] Implement secure memory wiping utilities to prevent key material from lingering after use.

### System utilities
- [ ] Broaden platform coverage in `Compatebility` shims, ensuring Windows, macOS, and Linux paths share identical semantics.
- [ ] Provide filesystem utilities (recursive copy, permissions inspection) that reuse `System_utils_file_*` primitives.
- [ ] Add resource-lifetime tracers that log when abort/assert handlers fire to aid crash diagnostics.
- [ ] Offer environment sandboxing helpers that capture and restore process state during tests.
- [ ] Build service management helpers (daemonization, signal handling) consistent across supported platforms.
- [ ] Document security hardening options (seccomp, chroot) for services built on top of these utilities.

### File and ReadLine modules
- [ ] Harmonize the File module with `ft_file` so both expose the same error-reporting surface and buffering strategies.
- [ ] Extend ReadLine to support customizable key bindings, history persistence, and syntax-aware completion callbacks.
- [ ] Add UTF-8 aware cursor movement and rendering in ReadLine to match the Unicode helpers in Libft.
- [ ] Provide pluggable history storage (SQLite, JSON) and search that respects multi-byte characters.
- [ ] Document terminal capability detection and fallback behaviour for minimal environments.

### Printf
- [ ] Bring the printf implementation up to C99 compliance (length modifiers, positional arguments, floating-point formatting).
- [x] Add sandboxed formatting tests that ensure buffer boundaries and return values match the standard library behaviour.
- [ ] Provide extension hooks for user-defined specifiers that integrate with `ft_string` and logging sinks.
- [ ] Optimize hot paths (integer/float formatting) using precomputed tables where possible.
- [ ] Document thread-safety guarantees and recommend locking strategies for shared formatters.
- [ ] Re-enable the `pf_vsnprintf matches std::vsnprintf output` regression after aligning `pf_vsnprintf` with the standard library results across truncation and zero-size scenarios.

### JSON / YAML / XML / HTML
- [ ] Share a common DOM and schema validation layer so parsers can reuse traversal logic across formats.
- [ ] Add streaming encoders/decoders that operate on callbacks to avoid holding entire documents in memory.
- [ ] Integrate schema evolution helpers (migrations, versioning) for configuration management.
- [ ] Expand XPath/CSS selector support in HTML and ensure namespace-aware parsing in XML.
- [ ] Provide tooling that round-trips documents and highlights diffs to simplify regression testing.
- [ ] Offer pluggable storage backends (file, HTTP, memory) for parsers to read from and serializers to write to.
- [ ] Publish style guides for canonical formatting so diffs remain readable across teams.

### Game module
- [ ] Build deterministic simulation tests that cover combat, crafting, quest progression, and event scheduling.
- [ ] Add save/load persistence using the Storage module so world state can be serialized and restored.
- [ ] Implement AI behavior trees or state machines for NPCs to replace ad-hoc logic.
- [ ] Profile the event scheduler under heavy load and optimize shared-pointer churn.
- [ ] Document extension points for mods or scripting integrations.
- [ ] Introduce scripting bridges (Lua, Python) with sandboxing controls for user-generated content.
- [ ] Add telemetry hooks that emit gameplay metrics for balancing and analytics.

### Tools and ancillary utilities
- [x] Provide a `make format` target that runs clang-format with a checked-in style file.
- [ ] Add developer ergonomics such as pre-commit hooks, git blame ignore lists, and automated code ownership updates.
- [ ] Create code-generation scaffolds (module boilerplate, test templates) that follow project conventions.
- [ ] Document how to bootstrap a development environment on macOS, Linux, and Windows Subsystem for Linux.

## Security and compliance
- [ ] Run dependency audits (e.g., `cargo audit`, `npm audit`, `pip-audit` equivalents) and track remediation timelines.
- [ ] Add threat models for networking, storage, and encryption modules, outlining mitigations for common attack vectors.
- [ ] Provide secure coding checklists for contributors, including guidelines for handling secrets and memory safety.

## Observability and operations
- [ ] Standardize logging levels and message formats across modules for consistent ingestion by external systems.
- [ ] Expose health endpoints or self-test routines that downstream services can call for readiness checks.
- [ ] Integrate tracing instrumentation (OpenTelemetry spans) across async boundaries for end-to-end visibility.
- [ ] Publish SLO dashboards that highlight latency, error, and saturation metrics for critical modules.

## Community and support
- [x] Create issue and PR templates that capture reproduction steps, environment info, and expected behaviour.
- [ ] Host regular changelog updates (blog posts, release highlights) to keep users informed about progress.
- [ ] Stand up a discussion forum or chat channel with moderation guidelines to support adopters.
- [ ] Offer office-hours or triage rotations to ensure community questions receive timely responses.

