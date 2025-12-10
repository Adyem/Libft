# Common Task Examples

Libft's modules share error-handling conventions built around `ft_errno` and
module-specific `_error_code` fields. The examples below illustrate how to
compose higher-level workflows while propagating failures consistently. Each
snippet is designed to be copy-paste ready and highlights when you should query
`ft_errno`, retrieve human-readable strings with `ft_strerror`, or consult
module accessors such as `get_error()`.

## File I/O: Copy a file and clean up temporary artifacts

This example uses the helpers declared in `File/file_utils.hpp` to copy a file
into a temporary directory, confirm success, and remove any stale artifacts on
failure.

```cpp
#include "../File/file_utils.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

bool copy_temporary_report(const char *source_path, const char *working_dir)
{
    if (!source_path || !working_dir)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }

    ft_string destination_path = file_path_join(working_dir, "report.txt");
    if (destination_path.c_str() == ft_nullptr)
    {
        // file_path_join already set ft_errno with the allocator failure.
        std::fprintf(stderr, "join failed: %s\n", ft_strerror(ft_errno));
        return (false);
    }

    if (file_copy(source_path, destination_path.c_str()) != 0)
    {
        std::fprintf(stderr, "copy failed: %s\n", ft_strerror(ft_errno));
        file_delete(destination_path.c_str());
        return (false);
    }

    if (!file_exists(destination_path.c_str()))
    {
        ft_errno = FT_ERR_NOT_FOUND;
        std::fprintf(stderr, "destination missing: %s\n",
            ft_strerror(ft_errno));
        return (false);
    }

    return (true);
}
```

Key points:

- `file_path_join`, `file_copy`, `file_exists`, and `file_delete` translate
  platform-specific failures through `Compatebility` and update `ft_errno` on
  your behalf.【F:File/file_utils.hpp†L1-L13】【F:File/file_copy.cpp†L1-L8】
- Always check the return value before trusting side effects; clearing up the
  destination ensures partially written files do not linger.

## HTTP client: Fetch JSON over TLS with retries

Synchronous helpers in `API/api.hpp` return dynamically allocated buffers.
Capture the HTTP status, inspect `ft_errno` on failure, and release the body
with `cma_free` when you are done.

```cpp
#include "../API/api.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

bool fetch_profile(const char *host, uint16_t port)
{
    if (!host)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }

    api_retry_policy retry_policy;
    retry_policy.set_max_attempts(3);
    retry_policy.set_initial_delay_ms(200);
    retry_policy.set_max_delay_ms(2000);
    retry_policy.set_backoff_multiplier(2);

    int status = 0;
    char *body = api_request_string_tls(host, port, "GET", "/v1/profile",
        ft_nullptr, ft_nullptr, &status, 10000, &retry_policy);
    if (!body)
    {
        std::fprintf(stderr, "request failed: %s\n", ft_strerror(ft_errno));
        return (false);
    }

    bool success = (status >= 200 && status < 300);
    if (!success)
        std::fprintf(stderr, "upstream error %d\n", status);

    cma_free(body);
    return (success);
}
```

Key points:

- `api_request_string_tls` stores transport failures in `ft_errno` via the
  request guard so your caller can branch on the precise error code.【F:API/api.hpp†L1-L156】【F:API/api_request_tls.cpp†L568-L846】
- Always release the response with `cma_free`, matching the CMA allocator used
  inside the API module.

## Task scheduling: Periodic work with cancellation

`ft_task_scheduler` exposes `_error_code` through both the scheduler instance
and the `ft_scheduled_task_handle` it returns for recurring work. Schedule a
periodic callback, poll `get_error()` for diagnostics, and cancel it when the
application shuts down.

```cpp
#include "../PThread/task_scheduler.hpp"
#include "../Errno/errno.hpp"
#include <atomic>
#include <cstdio>

void run_periodic_job()
{
    ft_task_scheduler scheduler_instance(2);
    if (scheduler_instance.get_error() != FT_ER_SUCCESSS)
    {
        std::fprintf(stderr, "scheduler init failed: %s\n",
            scheduler_instance.get_error_str());
        ft_errno = scheduler_instance.get_error();
        return ;
    }

    std::atomic<bool> keep_running(true);

    ft_scheduled_task_handle handle = scheduler_instance.schedule_every(
        std::chrono::seconds(5),
        [&keep_running]()
        {
            if (!keep_running.load())
                return ;
            // Execute the periodic task body here.
            return ;
        });

    if (!handle.valid())
    {
        std::fprintf(stderr, "schedule failed: %s\n", handle.get_error_str());
        ft_errno = handle.get_error();
        return ;
    }

    // ... later during shutdown
    keep_running.store(false);
    if (!handle.cancel())
    {
        std::fprintf(stderr, "cancel failed: %s\n", handle.get_error_str());
        ft_errno = handle.get_error();
    }
}
```

Key points:

- Scheduler constructors and scheduling helpers mirror `_error_code` into
  `ft_errno` whenever allocation, synchronization, or timer operations fail.
- Handles returned from `schedule_every` surface cancellation issues through
  `get_error()`/`get_error_str()`, allowing centralized logging and recovery
  flows.【F:PThread/task_scheduler.hpp†L89-L119】【F:PThread/task_scheduler.hpp†L136-L199】

## Working with the examples

Each snippet can live in a standalone translation unit. When integrating the
examples into your project:

1. Include the module headers relative to your build system's include roots.
2. Initialize subsystem globals (e.g., `ft_socket::initialize`) if the module
   requires explicit setup before use.
3. Route error codes to your logging or telemetry stack so operational tooling
   can surface regressions quickly.

By following these patterns you maintain consistent error semantics across
modules while giving callers copyable starting points for common workflows.
