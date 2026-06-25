# Module Dependency Cycle Resolution Options

This note captures the remaining `CPP_class` / `Template` / `PThread` layering issue and the main ways to resolve it.

## Current state

- The `ft_nullptr` stand-in has already been moved into `Basic`.
- The specific `PThread -> CPP_class` dependency through `class_nullptr` is gone.
- The `Template -> CPP_class` compile-probe edge is gone.
- The larger cycle still remains because:
  - `CPP_class` still depends on `PThread`, `Template`, and `Threading`
  - `Template` still depends on `PThread`
  - `PThread` still depends on `Template`
  - `Threading` still depends on `Template`

## Potential fixes

### 1. Move more low-level primitives out of `CPP_class`

Goal:
- Make `CPP_class` a consumer of core utilities, not a provider of them.

Likely candidates:
- cancellation primitives
- any mutex-adjacent helper that is really infrastructure
- any remaining utility that is not truly object-wrapper specific

Effect:
- Reduces `CPP_class -> PThread` pressure.
- Helps isolate `CPP_class` from the shared substrate.

Tradeoff:
- Least disruptive, but may not be enough on its own.

### 2. Split `Template` into two layers

Goal:
- Separate low-level value/container utilities from concurrency- or object-heavy adapters.

Suggested split:
- `TemplateCore`
  - `optional`
  - `variant`
  - `tuple`
  - `vector`
  - `map`
  - `sharedptr`
  - `uniqueptr`
  - `function` only if it remains lightweight
- `TemplateHigh`
  - concurrency-aware helpers
  - adapters that need `CPP_class`
  - wrappers that depend on `PThread`

Effect:
  - Lets low-level template utilities stay below `CPP_class`.
  - Removes the old `Template -> CPP_class` pressure.

Tradeoff:
- Larger refactor, but usually the cleanest architectural fix.

### 3. Move thread-related utilities out of `Template`

Goal:
- Stop `Template` from depending on `PThread`.

Likely suspects:
- `future`
- `promise`
- `event_emitter`
- queue or proxy types that lock directly

Effect:
- Makes `Template` a more purely value/container-oriented layer.
- Leaves concurrency concerns in `PThread`.

Tradeoff:
- This is often the most direct cut if the cycle is driven by concurrency helpers.

### 4. Reduce `CPP_class` to a true object-utility layer

Goal:
- Keep `CPP_class` focused on rich object wrappers instead of acting as core infrastructure.

What that means:
  - Prefer `Basic`, `CMA`, and `Errno` where possible for low-level support.
  - Use `PThread` only as a consumer-facing dependency, not as a provider of shared foundations.

Effect:
- `CPP_class` stops acting like part of the base substrate.

Tradeoff:
- More internal code churn, but cleaner boundaries.

### 5. Introduce an explicit substrate stack

Goal:
- Formalize the intended layering instead of relying on implicit structure.

Proposed stack:
- `Basic`
- `Errno`
- `ThreadCore`
- `CMA`
- `Containers`
- `CPP_class`
- `PThreadHigh`

Effect:
- Makes the dependency direction explicit.
- Gives each module a clearer place in the architecture.

Tradeoff:
- Biggest refactor, but strongest long-term outcome.

### 6. Enforce dependency direction mechanically

Goal:
- Prevent the cycle from being reintroduced after it is fixed.

Practical enforcement:
- maintain an allowlist per module
- fail CI if a module includes forbidden higher layers
- keep the dependency graph doc as a tracked contract

Effect:
- Stops regressions.

Tradeoff:
- This does not solve the existing cycle by itself.

## Recommended order

1. Keep `ft_nullptr` in `Basic`.
2. Audit `Template` for concurrency-facing utilities that really belong in `Threading`.
3. Move those pieces out first.
4. Then trim `CPP_class` so it no longer feeds core infrastructure back into the higher concurrency layers.

## Summary

The smallest credible path is to cut the `Template -> PThread` pressure first, because the direct `Template -> CPP_class` edge has already been removed. If that is not enough, then split `Template` and reduce `CPP_class` until the graph becomes acyclic.
