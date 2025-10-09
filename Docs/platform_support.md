# Platform Support and Compatebility Extension Guide

This document outlines the officially supported environments for Libft and explains how to extend the Compatebility shims when porting the library to new targets. It supplements the per-module overviews in `Docs/module_overviews.md` and the generated API reference.

## Supported platforms

Libft targets the following operating systems and architectures:

- **Linux**: glibc-based distributions on x86_64 and aarch64.
- **Windows**: Windows 10 and later on x86_64 using MSVC or clang-cl toolchains.
- **macOS**: macOS 12 Monterey and later on x86_64 and Apple Silicon.
- **BSD variants**: FreeBSD 13 and OpenBSD 7 on x86_64 when using clang.

Other Unix-like systems may work if they provide POSIX threads, BSD sockets, and C++17 compliant toolchains. These targets are considered best-effort and receive fixes as community contributions arrive.

## Compatebility shim overview

The Compatebility module hides platform-specific APIs behind consistent helper functions. Each shim follows the pattern below:

1. Provide a public header that exposes the cross-platform surface.
2. Implement platform-specific helpers in separate translation units placed under `Compatebility/`.
3. Update the module's manifest so `FullLibft.hpp` and build systems pull in the new files.
4. Track failures through `ft_errno` using the error-code registry in `Errno/errno.hpp`.

When adding a new platform, ensure the shim honors the root coding guidelines:

- Use snake_case identifiers and Allman-style braces.
- Avoid for loops, ternary operators, and switch statements.
- Provide constructors and destructors for any new classes, even if they simply contain `return ;`.

## Adding a new platform

When porting to a new operating system, use the following workflow.

### 1. Survey native APIs

Document which OS services back each Compatebility primitive (file system, sockets, threading, timers). Capture any behavioral differences such as blocking semantics or flag names. Place these notes in the module's README to guide future maintenance.

### 2. Create platform-specific helpers

For every primitive, create a dedicated implementation file. Example layout:

```
Compatebility/
    compatebility_file_posix.cpp
    compatebility_file_windows.cpp
    compatebility_file_newos.cpp
```

Share common code in internal helpers rather than sprinkling preprocessor checks throughout call sites.

### 3. Extend build configuration

Update CMake, Meson, and other provided templates so the new files compile conditionally. Provide a configuration option (e.g., `LIBFT_ENABLE_NEWOS`) that toggles the platform support and defaults to off until stable.

### 4. Add tests

Create regression tests that exercise the shim through its public interface. Focus on:

- Error code propagation (`ft_errno`).
- Resource cleanup paths (file descriptors, handles).
- Edge cases such as non-blocking sockets or unusual locale settings.

### 5. Document runtime requirements

List any additional dependencies (dynamic libraries, kernel settings) and how to install them. Include troubleshooting steps for common failures like missing headers or incompatible toolchains.

## Ongoing maintenance

- Schedule periodic CI runs on each supported platform. If CI is unavailable, perform manual smoke tests before releases.
- Keep the error-code registry synchronized with platform-specific failures so new codes are documented.
- Encourage community contributions by labeling issues with platform tags (`platform:newos`) and documenting the review checklist.

By following this guide, contributors can extend Compatebility while preserving a consistent cross-platform experience for Libft users.
