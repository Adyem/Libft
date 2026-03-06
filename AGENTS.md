#Coding Guidelines

Void functions must always end with `return ;` (space before `;`).
Non-void functions must return using `return (value);` (space before `(`).
Do not use for loops, ternary operators, or switch statements.
Indent code using 4 spaces per level.
After declaring a class, indent access specifiers (private, public, protected) by 4 spaces and indent member declarations within them by 8 spaces. This does not apply to structs.
In class declarations, place private members above public members and separate the sections with an empty line.
Function and variable names must use snake_case.
Constants and macros must use UPPER_SNAKE_CASE (all uppercase letters with `_` separators), for example `FT_ERR_SUCCESS`.
Do not use built-in width-ambiguous integer types such as `int`, `unsigned int`, `long`, `unsigned long`, `long long`, or `unsigned long long`.
Use project/fixed-width integer types instead (`ft_size_t`, `uint32_t`, `uint64_t`, `int32_t`, `int64_t`, and their signed/unsigned fixed-width variants).
Do not use built-in `bool`; use `ft_bool` (`typedef uint8_t ft_bool`) so boolean storage is always 8-bit.
Define boolean constants as `#define FT_FALSE ((ft_bool)0U)` and `#define FT_TRUE  ((ft_bool)1U)`.
Use full variable names instead of short ones or single letters, dont use s or str use string. Use names like file_descriptor and index; if multiple file descriptors or indexes are used, name each of them like file_descriptor_input, file_descriptor_output, index_height, index_width.

Use Allman style braces (opening brace on a new line).
Write `break` statements as `break ;` (a space between the keyword and the semicolon) to keep the spacing rules consistent.
In classes, member variable names must start with an underscore (_).
Within class member functions, access members and other methods using the this keyword.
For testing builds only, class private member variables must be exposed as `public` under `#ifdef LIBFT_TEST_BUILD` so tests can validate expected internal state directly. This must never be exposed in production builds.
Template classes may define member functions in the same header as the class declaration, but
other classes must split declarations into .hpp files and definitions into .cpp files. Inline and constexpr are allowed in headers for templates.
For template classes, the top section of the header must contain only the class declaration (member variables and method declarations).
Template member function bodies must not be defined inside the class declaration; they must be defined out-of-class in a separate section below the class declaration in the same header.
Do not define member function bodies inside the class declaration; place all definitions outside the class.
Every class must declare and define a default constructor and destructor. Do not use = default; explicitly define the bodies.
All classes must define copy and move constructors.
All classes must delete copy and move assignment operators by default. Only add assignment operators when a class explicitly needs assignment semantics and document that requirement. Operator overloading alone does not justify adding assignment operators.
Lifecycle classes must expose a dedicated explicit move helper named exactly `move` (for example `uint32_t move(ft_string& other) noexcept`) and keep this name consistent across classes.

## Class Skeleton Baseline

Use this baseline skeleton for classes (extend only when needed):

```cpp
typedef uint8_t ft_bool;
#define FT_FALSE ((ft_bool)0U)
#define FT_TRUE  ((ft_bool)1U)

class ft_example
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        static thread_local uint32_t _last_error;
        uint8_t _initialized_state;
        pt_recursive_mutex *_mutex;

    public:
        ft_example() noexcept;
        ft_example(const ft_example &other) noexcept;
        ft_example(ft_example &&other) noexcept;
        ~ft_example() noexcept;

        ft_example &operator=(const ft_example &other) noexcept = delete;
        ft_example &operator=(ft_example &&other) noexcept = delete;

        uint32_t initialize() noexcept;
        uint32_t initialize(const ft_example &other) noexcept;
        uint32_t initialize(ft_example &&other) noexcept;
        uint32_t destroy() noexcept;
        uint32_t move(ft_example &other) noexcept;

        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        static uint32_t set_error(uint32_t error_code) noexcept;
        static uint32_t get_error() noexcept;
        static const char *get_error_str() noexcept;
};
```

Notes:
- This skeleton is the default for lifecycle-style classes. Keep existing documented exemptions (proxy classes, `ft_nullptr` stand-ins, `Template/pair.hpp`, and `Template/function.hpp`).
- Use `pt_recursive_mutex *` for class-owned mutexes, never raw `pthread_mutex_t *`.
- Include `_last_error` helpers only for classes that expose `_last_error` behavior.

## Lifecycle, Error, and Thread-Safety Contract (Authoritative)

These rules are authoritative for lifecycle classes and override older conflicting wording.

Lifecycle scope:
- A lifecycle class is any class that can fail in isolation (for example, can return an error code, can fail allocation, can fail mutex/resource setup, or can otherwise enter a meaningful failure state on its own).
- Lifecycle enforcement is mandatory for all lifecycle classes except explicitly exempt categories listed below.
- Explicit exemptions:
  - Proxy classes are exempt from lifecycle and thread-safety ownership requirements; proxies must be designed so proxy operations themselves do not fail unexpectedly, while underlying class operations may fail and report errors explicitly.
    - A class is considered a proxy class only when its class name explicitly contains `proxy`.
  - `ft_nullptr`/`nullptr` stand-in utility classes are exempt from lifecycle enforcement and optional thread-safety helper requirements because they have no meaningful failure state and must complete their local task without failure.
  - `ft_nullptr` must explicitly behave as a null-pointer literal stand-in: assigning it to any pointer type must set that pointer value to `0` (null), equivalent to `nullptr`.

Lifecycle state:
- Classes with lifecycle management must keep an explicit 8-bit state field named exactly `_initialized_state` with type `uint8_t`:
  - `0`: uninitialized
  - `1`: destroyed
  - `2`: initialized
- Default constructors must never fail or abort.
- Constructors may zero/reset fields into a safe known uninitialized-ready state (`state = 0`), but must not perform fallible setup.
- Member-class objects must not be lifecycle-initialized in the constructor; they remain uninitialized until the owning class `initialize(...)` runs and initializes them explicitly.
- All fallible setup belongs in `initialize(...)`.
- Lifecycle class instances may be created on stack or heap; in both cases, callers are responsible for calling `initialize(...)` before first use.

Lifecycle error signaling:
- Add a dedicated not-initialized error code in the Errno module (for example `FT_ERR_NOT_INITIALIZED`) and use it consistently.
- Classes that expose `_last_error` must follow the single authoritative `#_last_error Contract` section below (including required prototype `set_error(uint32_t error_code)`).
- `get_error()` and `get_error_str()` called on uninitialized objects must report the not-initialized error code/string.

Shared lifecycle abort helper:
- Lifecycle abort formatting/behavior must be centralized in shared Errno internal helpers instead of per-class duplicated implementations.
- These helpers are for internal use only and must be declared in `Errno/errno_internal.hpp`.
- Required internal helper prototypes:
  - `void errno_abort_if_uninitialised(uint8_t initialized_state, const char *method_name);`
  - `void errno_abort_lifecycle(uint8_t initialized_state, const char *method_name, const char *reason);`
- Usage rules:
  - `errno_abort_if_uninitialised(...)` is used for the common guard path where a method requires `state == 2` and no custom reason text is needed.
  - `errno_abort_lifecycle(...)` is used when a specific reason must be attached (for example invalid source state in copy/move flows).
  - Classes must pass lifecycle state and method context into these helpers; classes must not format lifecycle abort messages locally.
- The helper prints a clear message to `stderr` and calls `su_abort();`.
- `su_abort();` is non-returning by contract: once called, execution must never continue in the caller path.

Initialize / destroy / destructor rules:
- `initialize()` aborts only when called while already initialized (`state == 2`) unless a class explicitly documents a different contract.
- Copy/move style initialization (`initialize(const T&)`, `initialize(T&&)`, copy/move constructors, explicit `move(...)`, and any explicitly-enabled copy/move assignment) must follow:
  1. Source must be initialized (`state == 2`) or lifecycle abort helper is invoked.
  2. `this == &other` is a no-op success.
  3. Destination may be uninitialized, destroyed, or initialized.
  4. If destination is initialized, call `destroy()` first and honor its return value.
- If initialization fails after partial work, the object must end in destroyed state (`state = 1`).
- `destroy()` must return error codes (no abort).
- If `destroy()` is called while the class is uninitialized (or already destroyed), it is a no-op and must return `FT_ERR_SUCCESS`.
- If the object is initialized and thread safety is enabled, `destroy()` must call `disable_thread_safety()` as the first cleanup step.
- `destroy()` must always continue cleanup of all owned resources even after a failure; if multiple failures occur, return only the first error code encountered.
- Destructors must fail silently (no abort), and are valid in uninitialized/destroyed states.
- For initialized objects, destructors must call `destroy()`; `destroy()` is responsible for running `disable_thread_safety()` first.
- Destructors must always run cleanup to completion: if intermediate steps fail, continue cleanup as best-effort and do not stop early.
- Destructor cleanup must leave class fields reset to an uninitialized-ready state (so `initialize(...)` could be called again in theory), even though destructor-time reinitialization should not be relied on.

Copy/move safety rules:
- For copy/move paths, perform all potentially failing work first.
- Only after success, apply non-failing state swaps/commits.
- Source remains unchanged until the failing phase is complete.
- On any copy/move failure (allocation failure, mutex/setup failure, or any reported sub-call failure), destination must be left in destroyed state (`state = 1`).
- For constructor-based copy/move, lifecycle state is the primary success/failure signal:
  - success => destination `_initialized_state == 2`
  - failure => destination `_initialized_state == 1`
  - Destroyed-state propagation is intentional for constructors: copy-constructing or move-constructing from a destroyed source (`state = 1`) is valid and must produce a destroyed destination (`state = 1`).
- Source state handling:
  - Source in uninitialized state (`state = 0`) is lifecycle misuse and must trigger lifecycle abort behavior.
  - Source in destroyed state (`state = 1`) is allowed for copy/move propagation paths; destination must end in destroyed state as well.
  - Source `_last_error` state is propagated as part of copy/move behavior when the class exposes `_last_error`.

Sub-object initialization:
- When a class initializes multiple sub-objects, it must call `initialize(...)` on each required sub-object.
- If one sub-object initialization fails, destroy all already-initialized sub-objects before returning.
- Parent object ends in destroyed state (`state = 1`) after failed initialization.

Thread-safety ownership and mutex requirements:
- Thread-safety support must be implemented if a class:
  - owns mutable shared state
  - allocates memory
  - modifies internal containers
  - may be accessed concurrently by callers
- Classes that only read immutable state may omit thread-safety support.
- `nullptr` utility/wrapper classes are an explicit exempt example.
- Optional thread-safe classes expose `enable_thread_safety()`, `disable_thread_safety()`, and `is_thread_safe()`.
- `_mutex` remains private and starts as `ft_nullptr`; `is_thread_safe()` is authoritative (`_mutex != ft_nullptr`).
- The mutex must be created/initialized only when the user explicitly calls `enable_thread_safety()`.
- Mutex allocation/ownership rule:
  - Class mutexes must always be recursive mutexes.
  - The mutex must always be heap-allocated.
  - For regular classes, the mutex is instance-local and must not be shared across instances.
  - Exception: the SCMA module may use one shared global mutex, and that global mutex must also live on the heap.

Enable/disable behavior:
- `enable_thread_safety()` must be safely retryable. If enable fails, object state remains valid for later retries.
- `disable_thread_safety()` destroys and deallocates the mutex and must set `_mutex = ft_nullptr` before returning, even when teardown reports an error.
- Mutex teardown must happen through `disable_thread_safety()`, including when called from `destroy()` or from destructor cleanup paths.
- If teardown fails, return the error code but still leave the class in a retry-safe state.

Locking rules:
- Use shared pthread safe lock/unlock helpers from `PThread/pthread_internal.hpp` for nullable mutex pointers; no RAII lock guards.
- Internal helper prototypes to use:
  - `int pt_mutex_lock_if_not_null(const pt_mutex *mutex_pointer)`
  - `int pt_mutex_unlock_if_not_null(const pt_mutex *mutex_pointer)`
  - `int pt_recursive_mutex_lock_if_not_null(const pt_recursive_mutex *mutex_pointer)`
  - `int pt_recursive_mutex_unlock_if_not_null(const pt_recursive_mutex *mutex_pointer)`
- For multi-object operations, always lock mutexes in ascending memory-address order (lowest address first) and unlock explicitly in reverse order.
- Unlock failures should be treated as non-recoverable usage-level issues: call sites may ignore unlock return values (`(void)unlock...`) after best effort and continue cleanup flow.

Dedicated testing-only mutex exposure helpers are not required.
Prefer test-build member exposure through `LIBFT_TEST_BUILD`; existing per-class mutex testing helpers should be removed unless a class explicitly documents a temporary exception.

## Pair Exception

`Template/pair.hpp` intentionally provides a minimal `Pair` template that exposes `key`/`value`
directly for compatibility with the rest of the library. That class is exempt from the mutex requirements
described above and must not introduce additional recursive mutex state or the associated helpers.
Treat `Pair` as a plain-old-data holder only.

#Template Function Simplification

`Template/function.hpp` defines `ft_function`. Keep this template as a lightweight callable wrapper
with no recursive mutex, errno helpers, or thread-safety toggles. Any new behavior should leave
error handling and synchronization to callers rather than reintroducing state inside `ft_function`.

## Deprecated Errno Stack

`ft_errno` and the legacy thread-local global error stack helpers (`ft_global_error_stack_*`,
`ft_error_stack`, `ft_operation_error_stack`, etc.) are deprecated. Do not add them back to any
new code. Remove references if you encounter them, and rely on return codes or other explicit error
reporting mechanisms instead.

## Error Code Definition Contract

All functions that return an error code must return one of the following only:
- `FT_ERR_SUCCESS`
- `FT_ERR_*`

Functions must never invent ad-hoc numeric error codes outside the Errno module.
Any new error code must be defined in the Errno `.hpp` file before use.

## Initialize Return Contract

`initialize(...)` methods must return `uint32_t` and follow the Error Code Definition Contract.


Only .cpp files must be prefixed with the name of the module they belong to. Nested modules must contain both the original module name followed by the nested module name.
For .hpp files, prefix only those meant for internal use with the module name.
Generic headers are headers used widely across the module that primarily collect many simple function prototypes and shared module-level declarations (module-wide API/utility/umbrella headers). These may use the module name (for example `math.hpp`, `networking.hpp`, `api.hpp`), while class headers should use the class name as the filename (for example `class_string.hpp`, `socket_handle.hpp`).

Update README.md only when the change is important for the end user, such as improved functionality or unavoidable undefined behavior. Bug fixes alone do not require README updates.

Code that relies on platform-specific features must place only the platform-dependent portions into helper functions in the Compatibility module. Platform-specific means the code cannot run on all of Linux, Windows, and macOS.

#Build and Test Timing

Building the library and running the full test suite typically takes about two and a half minutes, possibly a little longer.

#Allocation Failure Test Pattern

When using `cma_set_alloc_limit` in tests to force allocation failures:
- Use a non-zero limit to trigger failures (`0` means no allocation limit).
- Set the limit once at the start of the test (or test section), then run the failure scenario.
- Reset it exactly once to `0` at the end of the test (or section) to avoid leaking state across tests.
- Do not repeatedly set the same limit before and after a single call unless the test is explicitly stepping through multiple failure depths.

#Locking guidelines

This section defers to the authoritative `Locking rules` above.

#CMA Error Handling Rules

Within the CMA module, only the documented public-facing entry points should surface error signals to their callers; helper utilities should propagate failures through the module’s internal mechanisms.

#SCMA Error Handling Rules

Within the SCMA module, only the listed public-facing helpers should expose errors directly; all other utilities rely on the module’s internal reporting to keep overall state coherent.
SCMA classes do not need a per-class local mutex when the module-level global mutex already provides the required synchronization; prefer the SCMA global mutex in that case.
- `_last_error` access and behavior are defined only in the authoritative `#_last_error Contract` section below.

#`_last_error` Contract

When a class maintains a `_last_error` field, treat it as thread-local diagnostic state rather than a shared global-style error channel.
- `_last_error` must be thread-local per thread of execution. Code must not rely on a single shared `_last_error` value across threads.
- Declare `_last_error` as `static thread_local uint32_t _last_error` so each thread has its own copy while the class exposes a single helper suite.
- `_last_error` must be initialized to `0` (`FT_ERR_SUCCESS`) once at thread/program startup for that thread-local instance, not per object instance.
- Constructors and destructors must never modify `_last_error`.
- Classes that expose `_last_error` behavior must provide `uint32_t set_error(uint32_t error_code)`, `get_error()`, and `get_error_str()` helpers. `set_error(uint32_t error_code)` is the only writer for `_last_error`.
- `get_error()` / `get_error_str()` are the sanctioned public interfaces for reading class error state across classes when required. Other classes must not read `_last_error` directly.
- If a called method does not return an error code (for example `void` methods or value-returning methods without status), callers must read `get_error()` to obtain the operation error result.
- New code should still return error codes directly from methods that can fail. `_last_error` is a secondary diagnostic/compatibility channel, not the primary error transport.
- `_last_error` may be stale relative to earlier operations; lifecycle state is the authoritative indicator of constructor copy/move success.
- Only meaningful user-callable operational methods should set `_last_error` on every exit path (`FT_ERR_SUCCESS` on success, exact error code on failure). Private helpers should generally return errors upward and let the public entry point set `_last_error`.
