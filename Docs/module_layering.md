# Module Layering

This document gives a coarse architecture view of the library by grouping modules into layers.
It is not a hard build rule. Some modules intentionally straddle layers, and the detailed
truth always lives in [Docs/module_dependency_graph.md](module_dependency_graph.md).

## Layer 0: Absolute primitives
These modules provide the smallest shared building blocks and error codes.

- `Basic`
- `Errno`

## Layer 1: OS and raw primitives
These modules expose low-level platform, timing, file, threading, and random primitives.

- `PThread`
- `Time`
- `FileCore` equivalents inside `System_utils` and `Compatebility`
- `RandomCore` equivalents inside `RNG`
- `Compatebility`

## Layer 2: Memory
These modules own allocator and shared-allocator behavior.

- `CMA`
- `SCMA`

## Layer 3: Generic data structures
These modules provide reusable containers and generic template-based helpers.

- `Template`

## Layer 4: Core C++ conveniences
These modules add higher-level C++ wrappers and richer convenience helpers.

- `CPP_class`
- `Advanced`
- `Buffer`

## Layer 5: System utilities
These modules provide filesystem, logging, configuration, URI, and other reusable system-facing helpers.

- `System_utils`
- `File`
- `Filesystem`
- `Logger`
- `Config`
- `URI`
- `Sink`
- `ReadLine`

## Layer 6: Parsers and data formats
These modules parse, validate, and serialize structured text formats.

- `Parser`
- `JSon`
- `XML`
- `YAML`
- `CSV`
- `HTML`

## Layer 7: Services
These modules combine lower layers into reusable service-style building blocks.

- `Networking`
- `Encryption`
- `Storage`
- `API`
- `Observability`
- `Threading`

## Layer 8: Applications and domain logic
These modules are the most domain-specific surfaces built from the shared layers.

- `Game`
- `Voxel`
- `Application`
- `DUMB`
- `CLI`

## Notes on newer modules

- `Advanced` holds richer allocation-returning helpers that used to live in `Basic`.
- `Sink` is the low-level destination layer for log records and sits below `Logger`.
- `Threading` is the higher-level concurrency layer above `PThread`.
- `Buffer` is treated as a core convenience layer because it combines reusable storage with C++-style ownership helpers.

## How to use this map

- Read this layer map when you want a coarse “what should depend on what” picture.
- Read [Docs/module_dependency_graph.md](module_dependency_graph.md) when you need exact direct dependencies.
- If a module appears to straddle two layers, prefer the detailed dependency graph and the module README.
