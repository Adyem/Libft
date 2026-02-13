#Coding Guidelines

Use return ; for void functions. Include a space before the semicolon and always place a return ; at the end of every void function.
Use return (value); for non-void returns. Include a space before the opening parenthesis.
Do not use for loops, ternary operators, or switch statements.
Indent code using 4 spaces per level.
After declaring a class, indent access specifiers (private, public, protected) by 4 spaces and indent member declarations within them by 8 spaces. This does not apply to structs.
In class declarations, place private members above public members and separate the sections with an empty line.
Function and variable names must use snake_case.
Use full variable names instead of short ones or single letters, dont use s or str use string. Use names like file_descriptor and index; if multiple file descriptors or indexes are used, name each of them like file_descriptor_input, file_descriptor_output, index_height, index_width.

Use Allman style braces (opening brace on a new line).
Write `break` statements as `break ;` (a space between the keyword and the semicolon) to keep the spacing rules consistent.
In classes, member variable names must start with an underscore (_).
Within class member functions, access members and other methods using the this keyword.
Template classes may define member functions in the same file as the class declaration, but
other classes must split declarations into .hpp files and definitions into .cpp files. Inline and constexpr are allowed in headers for templates. For template classes, declare all member functions in the class declaration, then place the definitions below the class declaration in the same header.
Do not define member function bodies inside the class declaration; place all definitions outside the class.
Every class must declare and define a constructor and destructor, even if they simply contain return ;. Do not use = default; explicitly define the bodies.

## Thread-safety Lifecycle Contract

Every class that offers optional thread safety must provide the trio of explicit helpers `enable_thread_safety()`, `disable_thread_safety()`, and `is_thread_safe()`.
- `enable_thread_safety()` and `disable_thread_safety()` are the primary entry points for toggling synchronization. They must manage the mutex entirely on their own (e.g., `new`ing the mutex and calling its `initialize()` when enabling, destroying and deleting it when disabling) and cannot rely on other per-class helpers to bootstrap or tear down thread safety. They should return any underlying error code directly.
- `is_thread_safe()` should simply return whether the class currently owns a valid mutex pointer (i.e., `_mutex != ft_nullptr`), making it the single authoritative check for callers.
- Keep `_mutex` in the private section and start it as `ft_nullptr`; when `enable_thread_safety()` succeeds, the mutex pointer becomes valid, and `disable_thread_safety()` resets it to `ft_nullptr` before returning.

Locking rules:
- All locking and unlocking must call `this->_mutex->lock()`/`this->_mutex->unlock()` directly; do not hide the call behind RAII guards or additional wrappers. The moment thread safety is enabled, every method that mutates internal state should take the mutex directly and return any locking error immediately.
- Methods that operate on multiple objects (e.g., copying, moving, comparing two instances) must order the mutex pointers deterministically, lock them manually, and unlock them without relying on helper guards.

This contract keeps mutex management consistent across the codebase while ensuring that enabling/disabling thread safety is explicit and self-contained.

#Class Mutex Requirements

Each class must own a recursive mutex that can be locked multiple times and must be unlocked the same number of times.

All class definitions must be thread-safe by default; their internal state must be guarded by the class mutex so callers do not need to wrap instances in additional synchronization to get correct behavior.

Every class must expose a helper function that provides direct access to its recursive mutex.
Document and implement this helper as a dedicated low-level interface intended for cases like
validating constructor error handling immediately after construction by manually locking and
unlocking the mutex to verify proper use.
Because these helpers are only intended for testing and validation, guard their declarations
and definitions behind `LIBFT_TEST_BUILD` (or another dedicated testing-only macro); production
builds must never expose the recursive mutex or operation-stack handles publicly.

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

## Lazy Initialization Contract

Every class (particularly those that own mutexes) must only prepare its synchronization members in the constructor—real backing resources are created on-demand when `initialize()` is called. `destroy()` must be invoked explicitly before reusing or destructing the object again so the instance can return to an uninitialized state. This keeps object lifecycles predictable and matches the new `pt_mutex` contract.

## Initialization State Contract

For every class that has lifecycle methods, store lifecycle state in an explicit 8-bit field:
- `0`: uninitialized
- `1`: destroyed
- `2`: initialized

Behavior rules:
- Constructors must never abort, and must leave objects in the `0` (uninitialized) state.
- `initialize()` must abort only when called while already in state `2`.
- For copy/move style operations (`initialize(copy)`, `initialize(move)`, copy/move constructors, copy/move assignment, and explicit `move(...)` helpers):
  - the destination object may be in state `0` or `1` when the operation starts (it is valid to construct/initialize into an uninitialized destination),
  - the source object must be in state `2`; if the source is in state `0` or `1`, the operation must print the lifecycle error and abort.
  - Always apply checks in this order for copy/move paths:
    1. Validate the source object is initialized (`state == 2`), abort otherwise.
    2. Check `this == &other`; if true, treat as a no-op success.
    3. Continue the operation (including lazy-initializing the destination if needed).
- If any `initialize(...)` overload fails after it begins initialization work, it must leave the object in state `1` (destroyed), never in state `0`. This guarantees later destruction does not abort because of a failed initialization attempt.
- `destroy()` must abort when called in state `0` or `1`, and move state `2 -> 1` on success.
- All methods other than constructors and `initialize()` must abort when called in state `0` or `1`.
- Destructors must call `su_abort();` only when state is `0`. If state is `2`, they should destroy and move to `1`; if state is `1`, they must not abort.

Abort behavior is mandatory and uniform:
- Print a clear lifecycle error through the Printf module (for example `pf_printf_fd(2, "...\\n")`) with a trailing newline.
- Immediately call `su_abort();`.

Only .cpp files must be prefixed with the name of the module they belong to. Nested modules must contain both the original module name followed by the nested module name.
For .hpp files, prefix only those meant for internal use with the module name.
Generic headers may use the module's name, while class headers should use the class name as the filename.

Update README.md only when the change is important for the end user, such as improved functionality or unavoidable undefined behavior. Bug fixes alone do not require README updates.

Code that relies on platform-specific features must place only the platform-dependent portions into helper functions in the Compatebility module. Platform-specific means the code cannot run on all of Linux, Windows, and macOS.

#Build and Test Timing

Building the library and running the full test suite typically takes about two and a half minutes, possibly a little longer.

#Allocation Failure Test Pattern

When using `cma_set_alloc_limit` in tests to force allocation failures:
- Use a non-zero limit to trigger failures (`0` means no allocation limit).
- Set the limit once at the start of the test (or test section), then run the failure scenario.
- Reset it exactly once to `0` at the end of the test (or section) to avoid leaking state across tests.
- Do not repeatedly set the same limit before and after a single call unless the test is explicitly stepping through multiple failure depths.

#Locking guidelines

No function in the library should rely on RAII helpers such as lock guards or unique locks for locking mutexes; every call should explicitly lock and unlock the underlying mutex so there are no hidden dependencies on guard behavior. Use manual lock/unlock calls surrounding the protected section and handle errors directly—never cache guard objects to perform locking implicitly.

#CMA Error Handling Rules

Within the CMA module, only the documented public-facing entry points should surface error signals to their callers; helper utilities should propagate failures through the module’s internal mechanisms.

#SCMA Error Handling Rules

Within the SCMA module, only the listed public-facing helpers should expose errors directly; all other utilities rely on the module’s internal reporting to keep overall state coherent.
