# Compression Algorithm Evaluation

## Overview
To evaluate adding alternative algorithms behind the existing `Compression` interface, we surveyed
LZ4 and Brotli because they cover complementary performance envelopes. The goal was to determine how
to integrate them without breaking the current streaming abstractions and while preserving
portability across Linux, macOS, and Windows builds.

## Capability matrix
| Capability | LZ4 | Brotli |
| --- | --- | --- |
| Optimal use case | Real-time streaming, game state replication, IPC | Static asset packaging, HTTP content encoding |
| Compression ratio vs. zlib | ~0.75x | 1.1x–1.3x |
| Speed vs. zlib | 4x–6x faster | 0.6x–0.8x |
| External dependency footprint | Single C file or shared library | Shared library with encoder/decoder tables |
| License | BSD 2-Clause | MIT |
| SIMD/accelerated paths | SSE2/NEON ready | Portable, auto-detects SIMD |

## Integration approach
1. **Shared streaming hooks.** We can extend `t_compress_stream_options` with an `algorithm`
   field that defaults to `FT_COMPRESSION_ZLIB`. LZ4/Brotli entries can fan out to module-specific
   state machines but reuse the callback plumbing already exercised by the zlib implementation.
2. **Capability detection.** At build time the Makefile will probe for `liblz4`/`libbrotlienc` via
   pkg-config. Runtime detection falls back to zlib when the requested codec is unavailable,
   surfacing `FT_ERR_UNSUPPORTED_OPERATION` through `ft_errno` to keep error handling consistent.
3. **Compatibility shims.** When external dependencies are missing we can fall back to compiled-in
   minimal decoders. The plan is to vendor the official single-file LZ4 decoder for tests and gate
   Brotli behind optional targets so embedded builds avoid the heavier tables.
4. **API stability.** Existing callers continue to pass buffers/streams without modification. New
   helpers `compression_compress_buffer_ex`/`compression_decompress_buffer_ex` will accept the
   algorithm selector while preserving the legacy helpers as wrappers to avoid breaking changes.

## Testing strategy
* Extend `Test/Test/test_compression_stream.cpp` with fixtures that run LZ4/Brotli streams alongside
  the current zlib suite. Golden payloads derived from RFC 1951 fixtures guarantee byte-for-byte
  reproducibility.
* Add fuzz tests under `Test/fuzz/` that run mixed algorithm sequences to ensure the dispatch layer
  rejects unsupported switches and resets state machines appropriately.
* Populate a microbenchmark harness in `tools/bench_compression.py` to trend throughput for typical
  payload sizes (64 KiB, 1 MiB, 8 MiB) so regressions surface in CI dashboards.

## Rollout plan
1. Land the option plumbing and updated tests guarded behind `FT_ENABLE_EXPERIMENTAL_COMPRESSION`.
2. Stage CI runners with LZ4/Brotli packages enabled to ensure coverage without breaking minimal
   configurations.
3. After two release cycles of bake-in, promote the feature flag to on-by-default status and update
   `Docs/compression.md` with tuning guidance for workloads that prefer one codec over another.

## Outcome
The evaluation confirms that LZ4 offers immediate wins for latency-sensitive pipelines while Brotli
is well-suited to static asset distribution. The staged rollout keeps the API stable and gives us
room to monitor dependency health, satisfying the "Evaluate adding alternative algorithms" TODO item
and setting the groundwork for implementation work.
