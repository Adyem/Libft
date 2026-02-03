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
In classes, member variable names must start with an underscore (_).
Within class member functions, access members and other methods using the this keyword.
Template classes may define member functions in the same file as the class declaration, but
other classes must split declarations into .hpp files and definitions into .cpp files. Inline and constexpr are allowed in headers for templates. For template classes, declare all member functions in the class declaration, then place the definitions below the class declaration in the same header.
Do not define member function bodies inside the class declaration; place all definitions outside the class.
Every class must declare and define a constructor and destructor, even if they simply contain return ;. Do not use = default; explicitly define the bodies.

## Optional Thread-safety Contract

When a class exposes optional thread safety—as `ft_promise` does with `_thread_safe_enabled`/ `_mutex`—keep the same fields and contract in every class that follows the pattern (note: `ft_unique_lock` purposely opts out of this contract because it never enables per-instance thread safety, it simply proxies the wrapped mutex):
- Always declare `bool _thread_safe_enabled` and the associated pointer(s) (`pt_mutex *_mutex`, etc.) in the private section so they stay together with the rest of the synchronization state.
- Initialize `_thread_safe_enabled` to `false` and `_mutex` to `ft_nullptr`. Do not flip `_thread_safe_enabled` to `true` unless `prepare_thread_safety()` succeeds, and only allocate the mutex when it is needed.
- `prepare_thread_safety()` should allocate the mutex, check its error state immediately (via `operation_error_pop_newest()` / `ft_global_error_stack_pop_newest()`), set `_thread_safe_enabled = true` on success, and set `_thread_safe_enabled = false` plus release any resources on failure.
- Guard every lock/unlock helper with `_thread_safe_enabled`: if thread safety is disabled, return success without touching `_mutex`. When enabled, acquire/release `_mutex`, pop the newest error from the global stack, and propagate the real error value.
- `teardown_thread_safety()` must destroy/free `_mutex`, reset `_thread_safe_enabled` to `false`, and leave `_mutex = ft_nullptr` so repeated enable/disable cycles behave the same.
- Always call `prepare_thread_safety()` from constructors or enabling helpers before setting `_thread_safe_enabled = true`, and call `teardown_thread_safety()` from the destructor or disabling helpers before the object is destroyed so no dangling mutex remains.

Keeping this contract identical across classes makes it easy to reason about optional thread safety and prevents subtle bugs when enabling or disabling synchronization support.

#Class Mutex Requirements

Each class must own a recursive mutex that can be locked multiple times and must be unlocked the same number of times.

All class definitions must be thread-safe by default; their internal state must be guarded by the class mutex so callers do not need to wrap instances in additional synchronization to get correct behavior.

During error handling, do not lock the class mutex. Report errors directly to the thread-local global error stack instead of re-locking any mutex for Errno bookkeeping.

Every class must expose a helper function that provides direct access to its recursive mutex.
Document and implement this helper as a dedicated low-level interface intended for cases like
validating constructor error handling immediately after construction by manually locking and
unlocking the mutex to verify proper use.
Because these helpers are only intended for testing and validation, guard their declarations
and definitions behind `LIBFT_TEST_BUILD` (or another dedicated testing-only macro); production
builds must never expose the recursive mutex or operation-stack handles publicly.

## Recursive mutex lock/unlock contract
Every class that relies on an innate recursive mutex (mutexes should be thread-safe by default) must follow a strict error contract: when the class locks or unlocks its recursive mutex, first store the mutex return value, then pop the newest entry from the global error stack. Only push that entry back if it represents an actual failure; leave it removed when it signals success. The error code reported to `ft_errno` and mirrored on the class stack must prefer the global-stack entry but fall back to the mutex return when the popped entry was success, preventing redundant success entries while keeping real errors available for inspection. Classes that do not require a mutex for thread safety or are never used in a multithreaded environment are exempt.

## Pair Exception

`Template/pair.hpp` intentionally provides a minimal `Pair` template that exposes `key`/`value`
directly for compatibility with the rest of the library. That class is exempt from the mutex and
Errno requirements described above; it must not introduce a recursive mutex, error-stack tracking,
or the associated helpers. Treat `Pair` as a plain-old-data holder only.

#Template Function Simplification

`Template/function.hpp` defines `ft_function`. Keep this template as a lightweight callable wrapper
with no recursive mutex, errno helpers, or thread-safety toggles. Any new behavior should leave
error handling and synchronization to callers rather than reintroducing state inside `ft_function`.


#Errno Global Error Stack

Define and use a global error stack in the Errno module so non-class functions can record errors without overwriting prior state. All helpers operate on this per-thread stack; there is no additional locking requirement.

## Errno Module Global Stack API

The helper entries live in `Errno/errno.hpp`.

- `ft_errno_next_operation_id()` produces the next unique operation identifier.
- `ft_global_error_stack_push_entry(int error_code)` / `ft_global_error_stack_push_entry_with_id(int error_code, unsigned long long op_id)` push an entry and optionally reuse the provided ID so multiple stacks share the same entry.
- `ft_global_error_stack_push(int error_code)` pushes without caring about the ID.
- `ft_global_error_stack_pop_last()` / `ft_global_error_stack_pop_newest()` / `ft_global_error_stack_pop_all()` remove entries from oldest, newest, or the whole stack.
- `ft_global_error_stack_error_at(ft_size_t index)` / `ft_global_error_stack_last_error()` inspect recorded error codes without modifying the stack.
- `ft_global_error_stack_depth()` / `ft_global_error_stack_get_id_at(ft_size_t index)` / `ft_global_error_stack_find_by_id(unsigned long long id)` query the stack depth and locate entries by index or operation ID.
- `ft_global_error_stack_error_str_at(ft_size_t index)` / `ft_global_error_stack_last_error_str()` retrieve the human-readable string for recorded entries.

All callers that touch the global stack must leave the newest entry untouched if they did not push it themselves. If a caller pops an entry it pushed, it must re-push the same error or a success entry before returning so the stack remains consistent.

## Operation Stack Usage

Every class and module must rely solely on the thread-local `ft_global_error_stack` exported by the Errno module; do not track per-instance error histories in `ft_operation_error_stack`. This keeps all error state centralized and avoids duplicating identifiers.

If a public API needs to maintain module-local operation IDs (like CMA or SCMA), it must still correlate those identifiers with the global stack using helper functions such as `ft_global_error_stack_push_entry_with_id()`/`ft_global_error_stack_pop_entry_with_id()` so the same entry remains discoverable everywhere.

Only .cpp files must be prefixed with the name of the module they belong to. Nested modules must contain both the original module name followed by the nested module name.
For .hpp files, prefix only those meant for internal use with the module name.
Generic headers may use the module's name, while class headers should use the class name as the filename.

Update README.md only when the change is important for the end user, such as improved functionality or unavoidable undefined behavior. Bug fixes alone do not require README updates.

Code that relies on platform-specific features must place only the platform-dependent portions into helper functions in the Compatebility module. Platform-specific means the code cannot run on all of Linux, Windows, and macOS.

#Build and Test Timing

Building the library and running the full test suite typically takes about two and a half minutes, possibly a little longer.

#ReadLine Error Stack Rules

Within the ReadLine module, only public-facing entry points that callers use directly should push to the global error stack:
- rl_readline
- rl_clear_history
- rl_add_suggestion
- rl_clear_suggestions

All other ReadLine helpers are internal-use and must report errors via existing internal mechanisms without pushing to the global error stack.

#Compatebility Module Error Stack Rules

Everything in the Compatebility module is internal-use only and must not push to the global error stack directly. Instead, Compatebility helpers expose a module-scoped “last error” value that records the most recent failure or success without touching the global stack. Each helper must update that value when it reports an error, and callers are responsible for translating the recorded local error into the appropriate Errno code and pushing it onto the global stack before returning. This keeps the Compatebility code self-contained while ensuring upper layers still publish consistent error identifiers.

#Locking guidelines

No function in the library should rely on RAII helpers such as lock guards or unique locks for locking mutexes; every call should explicitly lock and unlock the underlying mutex so there are no hidden dependencies on guard behavior. Use manual lock/unlock calls surrounding the protected section and handle errors directly—never cache guard objects to perform locking implicitly.

#CMA Error Stack Rules

Within the CMA module, only public-facing entry points that callers use directly should push to the global error stack:
- cma_set_backend
- cma_clear_backend
- cma_backend_is_enabled
- cma_malloc
- cma_free
- cma_checked_free
- cma_strdup
- cma_strndup
- cma_memdup
- cma_calloc
- cma_realloc
- cma_aligned_alloc
- cma_alloc_size
- cma_block_size
- cma_checked_block_size
- cma_atoi
- cma_split
- cma_itoa
- cma_itoa_base
- cma_strjoin
- cma_strjoin_multiple
- cma_substr
- cma_strtrim
- cma_free_double
- cma_set_alloc_limit
- cma_set_thread_safety
- cma_get_stats
- cma_get_extended_stats

All other CMA helpers are internal-use and must report errors via existing internal mechanisms without pushing to the global error stack.

#SCMA Error Stack Rules

Within the SCMA module, only public-facing entry points that callers use directly should push to the global error stack:
- scma_initialize
- scma_shutdown
- scma_is_initialized
- scma_allocate
- scma_free
- scma_resize
- scma_get_size
- scma_handle_is_valid
- scma_write
- scma_read
- scma_get_stats
- scma_debug_dump
- scma_runtime_mutex
- scma_mutex_lock
- scma_mutex_unlock
- scma_mutex_close
- scma_mutex_lock_count

All other SCMA helpers are internal-use and must report errors via existing internal mechanisms without pushing to the global error stack.
