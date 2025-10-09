# 0003 - Manifest-driven umbrella includes

- **Status:** Accepted
- **Date:** 2024-02-18
- **Authors:** Header tooling maintainers
- **Supersedes:** N/A

## Context

`FullLibft.hpp` historically duplicated includes from multiple modules, occasionally falling out of sync when new headers were added. Manual maintenance introduced ordering bugs and conflicting macro definitions. Consumers depending on `FullLibft.hpp` could not rely on deterministic inclusion order, complicating incremental adoption.

## Decision

Generate `FullLibft.hpp` from the canonical manifest (`full_libft_manifest.txt`). A dedicated tool reads the manifest, sorts entries, and emits the umbrella include file during CI and release packaging. Modules only update the manifest when new headers are introduced, eliminating manual edits to the umbrella header itself.

## Consequences

- **Positive:** The umbrella header always reflects the manifest, preventing duplicates and maintaining alphabetical order. Changes are reviewable by inspecting the manifest diff.
- **Negative:** Local development environments require the manifest generation tool when headers change, adding a small dependency on Python tooling.
- **Follow-up:** The generator must run in CI to catch drift, and future refactors should extend the manifest format instead of modifying `FullLibft.hpp` directly.

## Considered Alternatives

1. **Hand-maintained includes:** Rejected because history showed repeated regressions.
2. **CMake configure-time generation:** Rejected to keep build-system agnosticism and avoid imposing CMake on non-CMake consumers.

## References

- `tools/generate_full_libft.py`
- `full_libft_manifest.txt`
- Release engineering runbooks
