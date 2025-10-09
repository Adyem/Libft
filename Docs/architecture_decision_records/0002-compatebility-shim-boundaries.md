# 0002 - Compatebility shim boundaries

- **Status:** Accepted
- **Date:** 2023-11-02
- **Authors:** Platform maintainers
- **Supersedes:** N/A

## Context

The `Compatebility` module shields platform-specific behaviour from the rest of the codebase. Before this ADR, several modules mixed Windows and POSIX branching directly in their implementations, which made porting difficult and encouraged subtle divergence. Contributors lacked guidance about where new platform code should live, leading to duplicated abstractions and poor discoverability.

## Decision

All operating-system-specific logic must reside within the `Compatebility` module. Other modules consume cross-platform helpers that expose a uniform API and return errors through the shared `ft_errno` registry. Platform detection and conditional compilation are centralized in `Compatebility`, which exports capability probes for higher-level code.

## Consequences

- **Positive:** Portability work focuses on the shim layer, reducing risk when adding new targets. Modules gain cleaner implementations that no longer depend on preprocessor checks.
- **Negative:** Some low-level optimizations that exploit OS-specific features now require additional abstraction plumbing through the shim APIs.
- **Follow-up:** Regular audits ensure that newly contributed modules respect the boundary. Documentation must stay current as shim capabilities evolve.

## Considered Alternatives

1. **Per-module platform hooks:** Rejected because it fragments knowledge and repeats error-handling patterns.
2. **Build-system plugins:** Rejected due to complexity and limited portability to downstream projects that vendor the library.

## References

- `Docs/platform_support.md`
- `Compatebility/` module headers
- Quick-start templates highlighting shim integration
