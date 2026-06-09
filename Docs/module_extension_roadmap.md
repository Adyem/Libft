# Module Extension Roadmap

This document collects concrete extension ideas for existing modules.
The goal is to grow the current module set by filling obvious gaps before
creating new top-level modules.

## General Guardrails

- Keep `Basic/` allocation-free and dependency-light.
- Put allocation-returning helpers, file-backed helpers, and other heavier
  convenience flows in `Advanced/` or a more specialized module.
- Prefer APIs that reuse the module's current error and lifecycle conventions
  instead of introducing a parallel style.

## Basic

Good additions here are small, non-owning helpers:

- Safe string copy helpers such as `strlcpy`/`strlcat` style functions.
- Prefix/suffix checks such as `starts_with` and `ends_with`.
- Substring containment checks and case-insensitive compare helpers.
- ASCII and UTF-8 boundary helpers, trimming helpers, and whitespace checks.

Do not add helpers that allocate memory, open files, or depend on other
subsystems here. Those belong in `Advanced/` instead.

## Advanced

This module is the right place for heavier string helpers:

- Title-case, snake_case, and camelCase conversion helpers.
- Whitespace normalization and string cleanup helpers.
- Join helpers for strings and vectors of strings.
- Safe formatting helpers that return allocated `ft_string` results.

## Buffer

Natural extensions for `Buffer/` include:

- Read and write cursor helpers for primitive types beyond the current endian
  helpers.
- Slice and view helpers.
- Buffer-to-string conversion helpers.
- `append_buffer`, `prepend_buffer`, and `shrink_to_fit` style operations.

## File

The file layer can absorb more filesystem workflows:

- Recursive directory copy and move helpers with filter callbacks.
- File hashing helpers for checksum workflows.
- Temporary-file helpers.
- Atomic write helpers.

## Filesystem

Useful path-centric additions here are:

- `is_rooted`, `is_hidden`, `is_reserved_name`, and `split_path` helpers.
- Better wildcard and glob matching.
- More edge-case handling in path normalization.
- Secure temp-path generation and root-containment helpers.

## Encoding

The encoding module can stay focused on transport-safe text transforms:

- Base32 helpers.
- URL-safe percent-encoding helpers.
- Streaming encode/decode variants.
- Stricter UTF-8 validation and normalization helpers.

## Compression

Compression can grow in the direction of stream-oriented workflows:

- Streaming compression and decompression for file descriptors or abstract
  sinks.
- Gzip and zlib wrapper helpers if they are not already present.
- Container-style helpers when the module is handling archive-adjacent flows.

## Errno

The error module is a good place for shared error infrastructure:

- Error categorization helpers for critical and recoverable outcomes.
- Shared lifecycle-state helpers.
- Centralized lifecycle abort formatting helpers.
- Additional error stringification and classification helpers.

## Template

The template layer can pick up more small generic algorithms:

- `emplace`, `remove_if`, `transform`, `filter`, and `flatten` style helpers.
- Iterator adapters and range-like utilities.
- More proxy helpers for chained access and error propagation.

## Time

The current timing helpers can be extended with:

- Date and time parsing and formatting helpers.
- Duration conversion helpers.
- Timezone-aware formatting helpers.
- Stopwatch and interval utilities for profiling.

## Logger

Good additions for logging are:

- Structured fields and scoped context propagation.
- File and buffer sinks.
- JSON or key-value formatting presets.
- Rate limiting and deduplication for noisy messages.

## Observability

The telemetry layer can grow with:

- Gauges and labeled counters.
- Timer scopes.
- Export adapters for logs, files, or telemetry backends.
- Span correlation helpers for `Threading/`, `Networking/`, and `Game/`.

## PThread

Good follow-ons here are synchronization conveniences:

- Thread join helpers.
- Condition-variable wait helpers, including timed waits.
- Reader/writer synchronization helpers.
- Diagnostics helpers for thread lifecycle and lock ownership.

## CLI

The command-line parser could add:

- Subcommand dispatch helpers.
- Command grouping helpers.
- Config-file merge helpers for CLI defaults.
- Richer validation helpers for argument constraints.

## Config

The config layer can extend its role as the runtime settings hub:

- Loaders for more config formats when appropriate.
- Validation-schema helpers.
- Default-merging helpers.
- Save, reload, and watch support for mutable runtime config.

## Prioritization Notes

- If a helper returns allocated memory, it should usually live in `Advanced/`
  rather than `Basic/`.
- If a helper primarily manipulates owned resources, keep it in the module that
  already owns those resources.
- If a helper becomes large enough to need distinct parsing, I/O, or platform
  logic, consider whether it belongs in a dedicated module instead of forcing it
  into an older one.

## Suggested First Pass

If the goal is to add value quickly without changing the overall module shape,
these are the best candidates to implement first:

1. `Basic/` prefix/suffix/contains and case-insensitive helpers.
2. `Advanced/` join and formatting helpers that return allocated `ft_string`
   values.
3. `Buffer/` slice, cursor, and append/prepend helpers.
4. `Filesystem/` containment and path-inspection helpers.
5. `Time/` duration conversion and formatting helpers.

These additions are low-risk because they extend existing concepts instead of
introducing new subsystem boundaries.
