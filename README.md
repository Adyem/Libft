# FullLibft

FullLibft collects the project's reusable C and C++ helper modules under a single build.
Each module below has a short purpose statement to clarify its role in the tree.

## Modules

- `API/`: Provides the glue between networking protocols, serialization formats, and business logic so clients and servers expose cohesive endpoints.
  This module hides low-level socket management, request routing, and payload marshaling behind helpers tailored to FullLibft patterns.
  It documents typical data flows such as accepting a request, parsing headers, and delivering structured responses reliably.
  API/ exists so other modules can mount services without re-implementing the repeating plumbing and error handling that networking requires.

- `CMA/`: Supplies a unified custom memory allocator layer that tracks statistics, enforces limits, and reports errors consistently across the tree.
Every module that allocates through this layer inherits the same mechanisms for throttling and alignment rules.
  CMA/ centralizes backend selection so switching between allocators or enabling debug instrumentation happens in one place.
  The goal is to shield the rest of the codebase from platform allocator quirks while still exposing enough hooks for tuning.

- `CPP_class/`: Wraps core C primitives in C++ classes, offering safer lifetime and ownership helpers without hiding the underlying semantics.
  It mixes in convenience helpers, move-aware containers, and RAII wrappers that keep the codebase modern yet lightweight.
  Each wrapper carefully translates the FullLibft naming/convention style into C++ idioms so consumers stay consistent.
  This module exists to let higher-level C++ projects build on the library without re-implementing their own bridging layers.

- `Compatebility/`: Keeps OS-specific code isolated so the library can stay portable across Linux, macOS, and Windows with minimal ifdefs.
  It provides consistent fallbacks and shims for filesystem details, locking primitives, and other APIs that vary between platforms.
  Rather than scattering platform checks in every module, Compatebility/ encapsulates the behavior and exposes clean, stable helpers.
  The net result is that higher layers can call uniform helpers and trust the module to handle quirks like different error codes or required structs.

- `Compression/`: Houses compression and decompression helpers for working with compact binary formats and reducing I/O costs.
  The code provides adapters for multiple algorithms while maintaining the FullLibft error and buffer patterns.
  Modules that ship data, store logs, or build snapshots rely on Compression/ to encapsulate detail about algorithm choice, streaming, and chunking.
  It exists to keep the rest of the project focused on data semantics instead of the mechanics of compressing or expanding byte streams.

- `Config/`: Loads and interprets configuration files, environment overrides, and runtime knobs that control FullLibft behavior.
  It understands different formats, merges defaults with overrides, and exposes typed accessors for modules to read settings safely.
  The module tracks modification timestamps and validation rules so configuration errors surface early in startup paths.
  Config/ guarantees there is a single source of truth for runtime state rather than duplicating parsing logic throughout the tree.

- `CrossProcess/`: Provides helpers for shared memory, interlocking primitives, and cross-process signaling so parallel services can coordinate.
  It wraps mutexes, semaphores, and event notification patterns into reusable abstractions that align with the shared allocator and error stack requirements.
  The helpers are designed for reliability when multiple processes contend for the same resources, handling cleanup and lifetime gracefully.
  CrossProcess/ exists to reduce the burden on modules that need inter-process coordination, letting them focus on the data being shared.

- `DUMB/`: Contains legacy or experimental helpers that prove useful for reference but are not part of the main sanctioned APIs.
  These helpers may have looser contracts, and they are kept separate to avoid introducing accidental dependencies for mainline code.
  DUMB/ doubles as a sandbox for prototypes that may graduate into other modules once their shape stabilizes.
  The folder reminds maintainers where compatibility shims or older implementations live so migration paths stay visible.

- `Docs/`: Captures design notes, module overviews, and user-facing documentation, keeping the project story aligned with the implementation.
  It includes notes on behavior guarantees, module interactions, and architecture diagrams that help contributors understand why each component exists.
  Docs/ ensures that knowledge does not live only inside source comments but is accessible through markdown that mirrors the code layout.
  Updating Docs/ keeps onboarding smoother and gives module authors a place to explain trade-offs or future directions.

- `Encryption/`: Supplies hashing, cryptographic primitives, and key handling utilities so security-sensitive modules share consistent implementations.
  The helpers cover symmetric/asymmetric hashes, HMAC, and key serialization logic tailored to the rest of the library's conventions.
  Modules that require authentication, signing, or secure communication build on Encryption/ so they avoid rolling their own cryptography.
  Its purpose is to provide vetted algorithms through FullLibft patterns rather than letting every module re-develop similar utilities.

- `Errno/`: Implements the shared global error stack, mutex wrappers, and the conventions that every class or module follows for reporting failure.
  It exposes helpers for pushing entries, querying depths, and mirroring results between the global and local stacks so errors stay consistent.
  Errno/ also documents the locking contract and operation ID generation that every consumer recreates when reporting issues.
  The module exists to keep the rest of the codebase synchronized on error identifiers and to simplify low-level error instrumentation.

- `File/`: Wraps filesystem interactions, path utilities, and attribute queries into safer helpers built on top of the standard C APIs.
  It understands common patterns such as reading/writing safely, walking directories, and handling special file types with predictable behavior.
  File/ ensures that the rest of the project does not repeat error handling for file descriptors or manual retries.
  By centralizing filesystem quirks, the module keeps higher layers focused on the data being stored rather than the underlying syscall dance.

- `Game/`: Contains game-specific data structures, simulation helpers, and server-side logic that model stateful interactions over shared primitives.
  It leverages core features like RNG, networking, and storage to deliver deterministic simulations and replay-friendly data formats.
  The module organizes rules, event queues, and validation routines so the flagship game projects have a stable foundation.
  Game/ exists to namespace game logic away from the engine utilities while still permitting reuse of general-purpose helpers.

- `Geometry/`: Offers vector math, shape handling, and transform helpers for building spatial reasoning layers on top of the library.
  It defines data structures for points, planes, and matrices along with functions for intersections, projections, and conversions.
  Geometry/ keeps the rest of the repository from recreating duplicated math when modeling positional data for games, networking, or storage.
  The goal is to provide battle-tested formulas and type-safe containers that can be reused across modules needing spatial math.

- `GetNextLine/`: Implements an incremental line reader that can consume arbitrary streams without buffering entire files into memory.
  The helpers manage internal buffers, handle partial reads, and expose a simple iterator-style interface for line-based workloads.
  This module is intended for tooling or services that parse line-delimited inputs without re-implementing the incremental logic each time.
  It exists to provide a drop-in replacement for repeated read/parse loops, ensuring consistent error handling in streaming scenarios.

- `HTML/`: Parses, models, and serializes HTML documents to support modules that operate on markup without reinvention.
  The module provides helpers for DOM traversal, attribute access, and safe serialization that respect the library error conventions.
  HTML/ also documents how to handle streaming inputs, sanitization, and tree modifications so other modules can focus on policy rather than parsing.
  It exists to keep markup processing centralized and consistent when multiple parts of the project generate or consume HTML.

- `JSon/`: Handles JSON parsing, validation, and serialization in a way that mirrors the rest of the library's style and error reporting.
  The helpers cover strict parsing, builder-style serialization, and helper macros that make structured data round-trippable.
  JSon/ lets callers define schemas and reuse typed accessors while leaving format details to the shared utilities.
  The module exists because the project routinely exchanges structured data and needs a single trusted implementation.

- `Libft/`: Provides the classic set of string, memory, and utility helpers inspired by the original Libft curriculum but expanded for modern C++.
  It includes normalized naming, allocator hooks, and consistent error reporting so every module can rely on the same baseline.
  Core helpers such as string duplication, memory comparison, and list splitting live here and underpin the rest of the codebase.
  Libft/ exists to anchor the project with a collection of well-known helpers that maintainers already understand and trust.

- `Logger/`: Implements logging sinks, formatting helpers, and routing logic so runtime events can be emitted in a consistent, configurable manner.
  It supports multiple log levels, structured contexts, and the ability to redirect output to files or other sinks.
  Logger/ funnels messages through the same error handling as other modules so log emission never surprises downstream observers.
  The module exists to centralize what would otherwise be a fragmented mess of printf and fprintf calls scattered across the tree.

- `Math/`: Supplies general math helpers, statistical routines, and linear algebra primitives used by computation-heavy modules.
  The helpers span basic arithmetic, vector operations, and numerical methods that are not specific to any single domain.
  Math/ keeps everyone from copy-pasting the same approximations, clamp functions, or distribution helpers while ensuring consistent naming.
  It exists to provide reliable implementations for performance-sensitive calculations without dragging in a heavyweight dependency.

- `Networking/`: Encapsulates sockets, HTTP helpers, and protocol utilities so modules that need to talk over the network can reuse the same foundations.
  This module understands connection lifetimes, framing, and serialization rules while integrating with the API and logging helpers.
  Networking/ hides port details, blocking modes, and error translation so callers can concentrate on request/response semantics.
  Its goal is to serve as the single source for transport code that other subsystems can tap without duplicating effort.

- `Observability/`: Provides metrics, tracing, and diagnostics utilities that give insights into runtime performance and behavior.
  The module follows the shared error conventions while exposing helpers that publish counters, histograms, and trace spans.
  Observability/ keeps samples and events in sync with the rest of the tree so instrumentation never feels like an afterthought.
  It exists to let service authors wire up telemetry without per-module experimentation, ensuring consistent visibility across deployments.

- `PThread/`: Wraps threading and synchronization primitives such as mutexes, condition variables, and thread creation helpers.
  Every helper normalizes platform differences and integrates with the error stack so concurrent modules share consistent safety practices.
  PThread/ exists to hide the raw pthread or platform APIs while letting callers still express locking intent directly.
  It ensures concurrency tools follow the same naming and lock/unlock rules mandated throughout the project.

- `Parser/`: Offers reusable parsing utilities, tokenizers, and grammar helpers that are consumed by format-specific modules.
  Parsers in this module expose consistent error reporting, lookahead strategies, and backtracking helpers for complex formats.
  Parser/ exists to avoid re-implementing lexing logic every time a new data format is introduced.
  By centralizing these pieces, the repository can share functionality between JSON, HTML, XML, and other format modules.

- `Printf/`: Builds consistent printf-style formatting helpers that respect buffer limits and thread-safety contracts.
  It exposes variadic helpers, format parsing routines, and destination-agnostic renderers used across other helpers.
  Printf/ exists to deliver deterministic formatting with the FullLibft error and allocator conventions.
  The module saves the rest of the codebase from juggling snprintf edge cases or recreated format parsers.

- `RNG/`: Holds random number generators, deterministic seeds, and distribution helpers that depend on the shared allocator and logging helpers.
  RNG/ provides multiple engines, entropy collection strategies, and helpers for shuffling or sampling data.
  It exists to keep simulations, games, and protocol flows synchronized with repeatable randomness without copy-pasting the same helper code.
  The module also exposes diagnostics to verify generator health when reproducing issues.

- `ReadLine/`: Implements readline-style interactive input helpers, command history, and suggestion helpers for terminal tooling.
  Its goal is to offer a predictable line-editing experience that integrates with the shared allocator, logging, and error stack rules.
  ReadLine/ handles character-by-character input, history persistence, and hooks for autocompletion so other modules can build REPLs.
  By centralizing the interface, the rest of the project avoids duplicating interactive input handling while still enjoying consistent behavior.

- `SCMA/`: Manages shared and secure memory allocation for scenarios where control over lifetime, growth, and instrumentation matters most.
  It layers on top of CMA/ but adds access control, snapshotting, and thread-safety guarantees required for shared memory use cases.
  SCMA/ exists to supply clients with predictable allocation limits, metadata tracking, and diagnostics for shared segments.
  The module is vital for any feature that needs coordinated access to chunks of memory across isolation boundaries.

- `Storage/`: Focuses on persistence, storage abstraction, and data handling utilities so storage behaviors are consistent across formats.
  Helpers manage serialization, checkpointing, and streaming data between memory and disk while respecting the error conventions.
  Storage/ exists to keep all persistence logic centralized so backups, caches, and durable objects share the same behavior.
  The module exposes hooks for custom backends, metadata tracking, and format-agnostic readers/writers.

- `System_utils/`: Contains OS wrappers, environment helpers, and runtime utilities such as process management and environment access.
  It unifies platform-specific behavior for fetching configurations, spawning subprocesses, or reading system properties.
  System_utils/ exists so modules do not duplicate logic for common tasks that depend on the current execution environment.
  The helpers keep the codebase in sync with lifecycle expectations without leaking platform conditionals everywhere.

- `Template/`: Hosts generic template-based utilities, containers, and type helpers shared across the C++ surface.
  It includes helpers for tuple-like behavior, type traits, and compile-time algorithms that respect the project's naming conventions.
  Template/ exists to provide reusable metaprogramming building blocks without coupling to other components.
  These templates help C++ modules remain expressive while still conforming to FullLibft error/logging contracts.

- `Test/`: Collects the test suite sources, runners, and helpers that validate module behavior before release.
  It provides harnesses for regression tests, fixture management, and command-line runners that mirror production expectations.
  Test/ exists to ensure every release candidate is exercised consistently with the rest of the project's configuration.
  The module also documents how to add new tests and interpret failure outputs so contributors can expand coverage confidently.

- `Time/`: Includes time and date helpers for scheduling, formatting, and measuring durations consistently.
  It wraps platform differences, exposes monotonic clocks, and formats timestamps following the library's conventions.
  Time/ exists to keep time-aware logic synchronized across modules that need consistent scheduling or logging semantics.
  The helpers prevent modules from duplicating conversions or misusing stopwatch-style timers.

- `XML/`: Provides XML parsing, document modeling, and serialization helpers focused on structured markup handling.
  This module outlines canonical parsing paths, namespace handling, and validation helpers for consistent tree manipulation.
  XML/ exists to keep the tricky aspects of XML (like entities and encodings) centralized for the rest of the project.
  Its helpers let other modules treat XML as a well-behaved data format without re-implementing the parser.

- `YAML/`: Handles YAML parsing, serialization, and validation to support human-readable configuration exchanges.
  The module exposes a schema-aware parser, emit helpers, and conversions that keep indentation and tags predictable.
  YAML/ exists to give modules a reliable way to consume or emit YAML while staying within the FullLibft style.
  It is especially useful for configuration, scripting, or metadata where YAML's readability is preferred.

- `tools/`: Houses developer scripts, build helpers, and documentation utilities that make building and testing easier.
  These scripts wrap repetitive commands, expose shortcuts for the toolchain, and keep the repository developer-friendly.
  tools/ exists to reduce friction when working on the project by giving quick access to linting, formatting, and release automation.
  Keeping these utilities isolated means contributors can evolve the workflows without touching the core library.
