# 0001 - Consolidated error code registry

- **Status:** Accepted
- **Date:** 2023-09-14
- **Authors:** Core maintainers
- **Supersedes:** N/A

## Context

Each module in Libft historically defined bespoke error enumerations. Callers who composed utilities from multiple modules needed to handle duplicated concepts (for example, multiple `OUT_OF_MEMORY` codes) and often lost fidelity when mapping failure states between components. The inconsistency also complicated documentation efforts and caused divergent semantics between the CMA wrappers and the core library.

## Decision

Adopt a canonical error code registry exposed through `Errno/errno.hpp`. Every module now references this registry to surface failures, and class-specific codes funnel into shared generic values whenever the semantics overlap. The registry is versioned and backed by a manifest so that new entries require coordination across modules before being published.

## Consequences

- **Positive:** Callers can write uniform error handling logic. Documentation now links to a single table rather than per-module lists, and regression tests ensure new codes remain unique.
- **Negative:** Introducing a new module-specific failure requires a registry update and cross-team coordination, adding upfront process overhead.
- **Follow-up:** Maintainers must keep the registry synchronized with module implementations and update migration guides when deprecating legacy codes.

## Considered Alternatives

1. **Per-module error namespaces:** Rejected because it exacerbated the confusion seen by downstream users and provided no path toward convergence.
2. **Opaque error objects:** Rejected due to increased binary size, ABI instability, and the desire to remain C-friendly for portions of the library.

## References

- `Errno/errno.hpp`
- `Docs/platform_support.md`
- `Docs/common_task_examples.md`
