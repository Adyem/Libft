# Libft FAQ and Troubleshooting Guide

This guide consolidates high-signal answers to recurring questions from issue reports and support requests. It pairs each question with concrete diagnostics so contributors can resolve problems quickly while maintaining the library's error-reporting conventions.

## General questions

### How do I surface detailed error information?
All modules propagate failures through `ft_errno`. Always read the error code immediately after a helper returns a failure indicator so subsequent calls do not overwrite it. When wrapping the library in higher-level abstractions, bubble both the code and the string returned by `ft_strerror(ft_errno)` to preserve context.

### Why do I see `FT_EINVAL` after calling a helper with seemingly valid data?
Most helpers validate preconditions aggressively to protect downstream code. Review the module overview for the helper you called and confirm every documented invariant: buffer sizes must match the documented length semantics, null terminators must be present for string helpers, and configuration structs must be zero-initialized before use. Violations of these preconditions translate to `FT_EINVAL` by design.

### What is the fastest way to determine which module set `ft_errno`?
Trace the call stack and correlate it with the error-code registry published in `Errno/errno.hpp`. Each entry documents the module responsible for the code. When in doubt, enable the optional logging hooks described in the module overviews; they emit a tagged message whenever `_error_code` changes, making it trivial to identify the originating module during debugging sessions.

## Build and integration issues

### My build fails because `FullLibft.hpp` references headers that do not exist. How do I fix this?
Regenerate the umbrella header using the manifest-driven generator:

1. Run `python tools/generate_full_libft.py` from the repository root.
2. Rebuild your project so the regenerated header is picked up.

If new headers were added without updating `full_libft_manifest.txt`, append them there so the generator includes them on the next run.

### Linking fails with undefined references to CMA allocation helpers. What should I check?
Ensure that your build links against both the Libft static library and the CMA module objects. When using CMake, include the `CMA` target alongside `Libft`. For manual builds, add the `.cpp` files under `CMA/` to your compilation units or compile them into a separate archive linked with your application.

### How can I integrate Libft into an existing build system quickly?
Use the minimal CMake template in `Template/` as a starting point. It demonstrates how to include the required directories, configure warnings, and link the core modules. For other build systems, replicate the include directories and compile definitions shown there.

## Runtime behaviour questions

### File I/O helpers report `FT_EIO` sporadically. Is this a bug?
`FT_EIO` indicates that the OS rejected the read or write request. Common causes include exhausted disk space, permissions mismatches, or transient hardware failures. Inspect the accompanying log entries (if logging hooks are enabled) and verify system-level diagnostics with `dmesg` or platform-equivalent tools. Retrying the operation without addressing the underlying system condition will typically fail again.

### HTTP client requests stall without timing out. How should I investigate?
Confirm that you configured the timeout fields on the request options structure. Without explicit values, the client honours system defaults, which can be indefinite on some platforms. If timeouts are set correctly, capture a trace with the networking module's observability hooks. They expose per-request state transitions, making it easier to pinpoint whether the stall occurs during DNS resolution, TLS negotiation, or response streaming.

### Why does the task scheduler drop work items under load?
The default scheduler prioritizes forward progress over backpressure. When the ready queue exceeds its internal limit, it rejects additional submissions and sets `_error_code` to `FT_EBUSY`. Monitor queue depth via the scheduler diagnostics and either tune the capacity in your configuration or shard workloads across multiple schedulers.

## Troubleshooting workflow

### Step 1: Reproduce with logging enabled
Enable per-module logging hooks (for example, `_api_logging` in the API module or `System_utils` trace toggles) to capture detailed state transitions. These logs provide the quickest path to understanding which invariant failed.

### Step 2: Collect error codes and context
Record the value of `ft_errno`, the helper name, and relevant parameters. Attach these details to bug reports so maintainers can reproduce the issue.

### Step 3: Minimize the test case
Reduce the scenario to the smallest reproducer possible. The `Template/` directory contains lightweight harnesses you can adapt for this purpose, allowing you to isolate the failing module without application overhead.

### Step 4: Verify environment assumptions
Ensure that environment variables, file-system paths, network endpoints, and platform-specific prerequisites (such as `WSAStartup` on Windows) meet the documented expectations. Many reported issues ultimately stem from mismatched environments.

### Step 5: Escalate with diagnostics
If the issue persists, open a ticket and include:

- Collected logs and error codes.
- Platform details (OS, compiler, architecture).
- A reproducible test case or step-by-step instructions.

Providing this information accelerates triage and reduces back-and-forth communication.

## Additional resources

- `Docs/module_overviews.md` – Design goals, invariants, and error-reporting patterns per module.
- `Docs/common_task_examples.md` – Copy-ready snippets showing correct `_error_code` handling.
- `Docs/platform_support.md` – Supported targets and extension pathways for new platforms.
- `Template/` – Ready-to-build integration samples for popular build systems.

