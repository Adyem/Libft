# FullLibft

FullLibft collects the project's reusable C and C++ helper modules under a single build.
Each module below has a short purpose statement to clarify its role in the tree.

## Modules

- `API/`: Client/server helpers that combine networking, parsing, and serialization into higher-level endpoints.
- `CMA/`: Custom memory allocation layer that standardizes allocation, limits, and error reporting across modules.
- `CPP_class/`: C++ class wrappers and utilities that lift the C foundations into safer object-oriented APIs.
- `Compatebility/`: Platform-specific shims that isolate OS differences so higher layers stay portable.
- `Compression/`: Compression and decompression utilities that handle compact data storage and transport.
- `Config/`: Configuration file loading and parsing helpers for project and runtime settings.
- `CrossProcess/`: Inter-process communication helpers for shared memory, locks, and cross-process signals.
- `DUMB/`: Legacy or experimental helpers retained for compatibility and prototyping.
- `Docs/`: Project documentation, design notes, and module overviews.
- `Encryption/`: Cryptography, hashing, and secure key utilities used by other subsystems.
- `Errno/`: Shared error stack, errno helpers, and error-reporting conventions for the library.
- `File/`: File and filesystem helpers that sit above the C standard library wrappers.
- `Game/`: Game data structures, simulation logic, and server utilities built on shared primitives.
- `Geometry/`: Geometry and spatial math primitives for vectors, shapes, and transforms.
- `GetNextLine/`: Line-reading helper module for incremental stream processing.
- `HTML/`: HTML parsing, document modeling, and serialization helpers.
- `JSon/`: JSON parsing, validation, and serialization utilities for structured data.
- `Libft/`: Core C-style string, memory, and utility helpers that anchor the project.
- `Logger/`: Logging utilities, sinks, formatting helpers, and log routing behaviors.
- `Math/`: Math helpers, statistics, and linear algebra primitives used across modules.
- `Networking/`: Socket, HTTP, and protocol helpers that implement network-facing features.
- `Observability/`: Metrics, tracing, and diagnostics utilities for runtime insight.
- `PThread/`: Threading and synchronization wrappers that normalize concurrency primitives.
- `Parser/`: General-purpose parsing utilities shared by format-specific modules.
- `Printf/`: Custom printf-style formatting helpers with consistent formatting behavior.
- `RNG/`: Random number generators and distribution helpers for deterministic or stochastic workflows.
- `ReadLine/`: Readline-style interactive input helpers and history handling.
- `SCMA/`: Shared/secure memory allocation module for controlled lifetime management.
- `Storage/`: Persistence, storage abstraction, and data handling utilities.
- `System_utils/`: System-level helpers, OS wrappers, and environment utilities for runtime integration.
- `Template/`: Generic templates, containers, and type utilities used across the C++ surface.
- `Test/`: Test suite sources and test runner helpers that validate module behavior.
- `Time/`: Time and date utilities for scheduling and formatting.
- `XML/`: XML parsing and serialization helpers for structured markup.
- `YAML/`: YAML parsing and serialization helpers for configuration and data exchange.
- `tools/`: Developer tooling scripts for builds, tests, and documentation tasks.
