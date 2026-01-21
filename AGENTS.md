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

#Class Mutex Requirements

Each class must own a recursive mutex that can be locked multiple times and must be unlocked the same number of times.

During error handling, do not lock the class mutex. Use the Errno module mutex wrapper so error reporting does not re-lock the class mutex.

Every class must expose a helper function that provides direct access to its recursive mutex.
Document and implement this helper as a dedicated low-level interface intended for cases like
validating constructor error handling immediately after construction by manually locking and
unlocking the mutex to verify proper use.

#Errno and Error Stack Rules

Class and non-class error handling must use the shared error-stack types from the Errno module.
Classes must push errors to both the thread-local error stack and their class-only error stack.

When a class method pushes an error, it must generate a single error entry containing a unique identifier.
This same error entry must be pushed to both the thread-local error stack and the class-only error stack
to ensure that the same operation has the same ID across both stacks.

When publishing or reading class error codes or error stacks, do not lock the class mutex; use the Errno mutex wrapper instead.

Errors and successful completions are reported by pushing entries onto the appropriate error stack.
A function that pushes an entry must leave it on the stack.
The function that checks the entry is responsible for popping it.
Functions marked strictly for internal use must not push any errors on the stack. Internal functions are typically marked static or declared in internal headers (headers with "internal" in their name).
When a function returns an error value, check that return value alone. Use the error stack only to
determine the error type (still pop the newest value when required).
If a function returns an error value and the popped error entry is success, do not publish or set
an error code from that success value.

If a function completes with an error entry still present on the stack,
it must not push a success entry.

If an error entry was reported and then popped because it was handled,
the function must push a success entry to report that it completed successfully.

If a function completes successfully without having pushed any entry,
it must push exactly one success entry.

Functions that report error text or terminate the process (such as ft_strerror, ft_perror, ft_exit, or any helper that exits)
must not modify the error stack.

#Errno Global Error Stack

Define and use a global error stack in the Errno module so non-class functions can record errors
without overwriting prior state, preserving older errors in the stack. Use helper functions for
all global stack access; do not touch the stack directly. Helpers must provide push, pop last,
pop newest, pop all, and fetch by index (1-based). The newest error is always at the lowest index,
and helpers must also support retrieving the most recent error and error string values using the
same ordering rules.

## Errno Module Global Stack API

The Errno module owns the thread-local global error stack. Every interaction with that stack
must happen while holding the Errno mutex wrapper so unrelated classes or helpers cannot race
when reporting. The mutex and helper layers live in `Errno/errno_internal.hpp`.

- `ft_errno_mutex()`: wraps the recursive mutex guarding the error stack.
- `ft_errno_next_operation_id()`: produces the next unique operation identifier.
- `ft_global_error_stack_push_entry(int error_code)` / `ft_global_error_stack_push_entry_with_id(int error_code, unsigned long long op_id)`: push an entry and optionally reuse the provided ID so multiple stacks share the same entry.
- `ft_global_error_stack_push(int error_code)`: convenience when the ID does not matter.
- `ft_global_error_stack_pop_last()` / `ft_global_error_stack_pop_newest()` / `ft_global_error_stack_pop_all()`: remove entries either from the oldest, newest, or entire stack.
- `ft_global_error_stack_error_at(ft_size_t index)` / `ft_global_error_stack_last_error()`: inspect recorded error codes without modifying the stack.
- `ft_global_error_stack_depth()` / `ft_global_error_stack_get_id_at(ft_size_t index)` / `ft_global_error_stack_find_by_id(unsigned long long id)`: query stack depth and locate entries by index or operation ID.
- `ft_global_error_stack_error_str_at(ft_size_t index)` / `ft_global_error_stack_last_error_str()`: retrieve the human-readable string for recorded entries.

All callers that push or pop entries from the global stack must leave the newest entry untouched if they did not push it themselves. If a caller pops an entry that it pushed, it must re-push either the same error or a success entry before returning, depending on the outcome.

## Local Operation Stacks (classes, CMA, SCMA)

Every class and module that tracks its own errors must reuse the `ft_operation_error_stack` model exported by the Errno module:

- Declare a thread-local `ft_operation_error_stack` instance dedicated to that class or module.
- Document a low-level helper that exposes direct access to the stack so callers writing low-level checks can manually lock, inspect, and unlock it during constructor validation.
- When reporting an error, first call `ft_errno_next_operation_id()` to reserve a unique identifier; then, while holding `ft_errno_mutex_wrapper`, push the entry on the Errno global stack with `ft_global_error_stack_push_entry_with_id(error_code, operation_id)` and immediately push the same (error_code, operation_id) tuple onto the class/module stack via `ft_operation_error_stack_push(stack, error_code, operation_id)`. This keeps both stacks synchronized on a single entry ID.
- When removing entries, use `ft_operation_error_stack_pop_last`, `_pop_newest`, or `_pop_all` before or after mirroring the removal on the global stack so that a popped entry is discarded from both locations.
- Inspect local stacks with `ft_operation_error_stack_error_at`, `_last_error`, and `_last_id` instead of duplicating logic.
- Every class should expose helper functions that let callers inspect and push/pull from its local stack in the same way they already expose the recursive mutex helper.
- During error handling, continue using the Errno mutex wrapper whenever the global stack is involved so the class mutex is not re-entered unintentionally.

_Note: `ft_nullptr_t` is a stateless sentinel and does not track errors via a per-class stack or push entries to the global stack; it never alters any error stacks._

Public CMA and SCMA entry points (see their dedicated modules for the authorized function lists) also maintain module-local `ft_operation_error_stack` instances. They follow the same pattern as class stacks: each public function pushes to both the Errno global stack and the module stack using the shared operation ID so that the error ID stays consistent across both stacks. Internal helpers inside CMA and SCMA remain on their dedicated stacks and communicate with callers via the local stack without touching the global Errno stack directly.

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

Everything in the Compatebility module is internal-use only and must not push to the global error stack directly.

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
- cma_leak_detection_enable
- cma_leak_detection_disable
- cma_leak_detection_clear
- cma_leak_detection_is_enabled
- cma_leak_detection_outstanding_allocations
- cma_leak_detection_outstanding_bytes
- cma_leak_detection_report

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
- scma_snapshot
- scma_release_snapshot
- scma_get_stats
- scma_debug_dump
- scma_runtime_mutex
- scma_mutex_lock
- scma_mutex_unlock
- scma_mutex_close
- scma_mutex_lock_count

All other SCMA helpers are internal-use and must report errors via existing internal mechanisms without pushing to the global error stack.
