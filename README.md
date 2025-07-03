# Full Libft

Full Libft is a collection of C++ libraries that provide many utilities used across
multiple projects.  It bundles reimplementations of C standard functions,
custom memory management, formatted output, networking helpers and more.  Each
feature lives inside its own directory and can be built on its own or combined
into a single static library.

The goal of this repository is to provide a portable set of building blocks that
can be dropped into a variety of projects.  Every module is self-contained with
minimal external dependencies, making it easy to reuse just the portions you
need.  Platform-specific code resides in dedicated folders so the same codebase
can be compiled on Linux, macOS or Windows without modification.

## Quick Start

1. Clone the repository and move into it:
```bash
git clone https://github.com/yourname/Libft.git
cd Libft
```
2. Build the library with `make` (use `make debug` for a debug version).
3. Compile your project including `FullLibft.hpp` and link against `Full_Libft.a`.
4. Optionally run the tests with `make -C Test` to verify the build.


## Building

Run `make` in the repository root to create `Full_Libft.a`.  Use `make debug`
for a debug build, `make both` to build both variants, and `make re` to clean
and rebuild everything.

```bash
$ make       # build release library
$ make debug # build debug library
```

The top-level Makefile compiles each module as its own static archive before
combining them into the final library.  If you only need a specific component
you can run `make` inside that subdirectory to produce its archive in
isolation.  The build system automatically selects the Linux or Windows sources
depending on your platform.

## Tests

A simple test suite resides in the `Test` folder.  Build and run it with:

```bash
$ make -C Test
$ ./Test/libft_tests
```

Running the suite will exercise a handful of standard library replacements and
ensure the static archives were linked correctly.  You can pass extra compiler
flags via `COMPILE_FLAGS` when invoking `make` in the `Test` directory if you
wish to experiment with additional warnings or sanitizers.

## Using the library

Link the produced `Full_Libft.a` (or the debug variant) against your project
and include the top level `FullLibft.hpp` header.  A typical compile line
might look like:

```bash
g++ -I/path/to/Libft main.cpp Full_Libft.a -o myprog
```

Each subdirectory also provides a standalone Makefile if you want just a single
component.  The library targets C++17 and relies only on the standard compiler
available on your platform.

## Modules

- **CMA** – Custom memory allocator.
- **GetNextLine** – Line-reading utility.
- **Libft** – Reimplementations of common C functions.
- **Printf** – Formatted output similar to `printf`.
- **ReadLine** – Interactive line reader.
- **PThread** – Thread helpers.
- **CPP_class** – Additional C++ classes.
- **Errno** – Error utilities.
- **Networking** – Cross-platform socket helpers.
- **Linux** and **Windows** – OS specific features.
- **encryption** – Encryption helpers.
- **RNG** – Random number generation.
- **JSon** – JSON parser and writer.
- **file** – File system helpers.
- **HTML** – HTML parser.
- **Template** – STL-like containers and utilities.

Every module contains its headers and source code so you can use them
independently or as part of the combined `Full_Libft` library.

## Module Details

Below is a more in depth look at the major modules and some of the
functionality they provide.

### CMA – Custom Memory Allocation

`CMA` implements a minimal allocator and a set of helper routines for
working with dynamically allocated memory.  The primary entry points are:

- `cma_malloc`, `cma_calloc`, `cma_realloc` and `cma_free` – drop in
  replacements for the standard C allocation calls.
- `cma_strdup` and `cma_memdup` – convenience functions for duplicating
  strings or memory blocks.
- `cma_split`, `cma_strjoin`, `cma_substr` and `cma_strtrim` – utilities
  for manipulating strings allocated by the CMA system.
- `cma_itoa` and `cma_itoa_base` – integer to string conversion helpers.
- `cma_free_double` – frees null terminated string arrays.
- `cma_cleanup` – releases any globally allocated resources.

The allocator tracks each allocation internally so debug builds can detect
memory leaks and invalid frees.  It aims to be small and predictable rather
than fast, making it useful in constrained environments or educational
projects.

### GetNextLine – Line Reading

The `GetNextLine` module exposes a convenient `get_next_line` function
for buffered input processing.  Additional helpers allow loading entire
files:

- `get_next_line(ft_file &file)` – returns the next line as a newly
  allocated string.
- `ft_read_file_lines(ft_file &file)` – reads all lines from an already
  opened file object.
- `ft_open_and_read_file(const char *filename)` – utility that opens a
  path and returns all of its lines.

The function maintains a small internal buffer allowing it to read from any
file descriptor without repeatedly allocating memory.  Because it operates on
the `ft_file` class it behaves consistently whether the underlying file is
from a regular file, a pipe or a network socket.

### Libft – C Utility Reimplementations

`Libft` contains lightweight replacements for many common C library
functions.  Examples include `ft_strlen`, `ft_atoi`, `ft_isalpha`,
memory operations such as `ft_memcpy` and string search helpers like
`ft_strchr`.  These functions mimic the behaviour of their standard
counterparts while conforming to the project coding style.

They are implemented from scratch for portability and as an educational
exercise.  The code avoids compiler-specific extensions so it can serve as a
drop-in replacement on systems where the C library might be missing features or
when static linking is desired.

### Printf – Formatted Output

The `Printf` folder implements a small `pf_printf` function compatible
with the `printf` format string syntax.  It supports the most common
format specifiers and also provides `pf_printf_fd` for writing to an
arbitrary file descriptor.

While not a complete reimplementation, `pf_printf` covers the placeholders most
often encountered in small tools and demos.  It is tiny enough to compile
quickly yet flexible enough for debugging output in larger applications.

### ReadLine – Interactive Input

`ReadLine` offers a simple line editor for terminal applications.  The
public API consists of:

- `rl_readline(prompt)` – read a line from standard input while handling
  arrow keys and history.
- `rl_add_suggestion(word)` and `rl_clear_suggestions()` – manage tab
  completion entries.
- `rl_clear_history()` – erase the stored input history.

Despite its simplicity the editor recognises common control sequences and
supports a pluggable completion mechanism.  History is stored in memory for the
duration of the session, providing a comfortable interface for quick utilities
and debugging tools.

### PThread – Threading Helpers

This module wraps a few POSIX thread primitives.  Functions such as
`pt_thread_create` and `pt_thread_join` mirror `pthread_create` and
`pthread_join`.  A lightweight `pt_mutex` class implements a mutex with
try-lock and error reporting features.

Where possible the wrappers fall back to C++ standard threading utilities when
POSIX threads are not available, giving the same API on Windows and other
platforms.

### CPP_class – Helper Classes

`CPP_class` provides some small utility classes used across the
library:

- `ft_file` – RAII wrapper around a file descriptor with methods for
  reading, writing and formatted output.
- `ft_string` – a dynamically sized string class supporting appending and
  direct C-string access.
- `data_buffer` – a simple growable buffer for binary data.

Error handling is built into these classes.  Each object stores its last
failure code and exposes accessor methods so callers can inspect what went
wrong.  For example `ft_file` provides `get_error_code()` and
`get_error_message()` while `ft_string` offers `getError()` and `errorStr()`.
`DataBuffer` maintains an internal flag queried with `good()` or `bad()`.  When
an operation fails these methods also update `ft_errno`, ensuring a consistent
error reporting mechanism across the library.

These helpers are intentionally lightweight so they can be embedded in small
projects without pulling in the full C++ standard library.  They interact
seamlessly with the other modules and provide a convenient foundation for more
advanced abstractions.

### Errno – Error Handling

Defines the `ft_errno` variable and a large set of library specific
error codes.  Use `ft_strerror` to translate an error code into a
message and `ft_perror` to print a formatted error string.

A uniform error system keeps reporting consistent throughout the library.  Each
module sets `ft_errno` when it encounters a failure, allowing callers to check
for problems without relying on platform specific `errno` values.

### Networking – Cross Platform Sockets

This module contains wrappers for basic socket operations along with two
helper classes:

- `SocketConfig` – stores configuration used when creating sockets.
- `ft_socket` – RAII socket wrapper supporting client/server setup,
  non-blocking mode, broadcasting and more.  Helper functions such as
  `nw_bind`, `nw_listen` and `nw_accept` directly expose the underlying
  system calls.

Both classes keep track of errors internally.  Methods like
`SocketConfig::getError()` and `ft_socket::get_error()` return the last
error code encountered while `get_error_message()` converts it to a
human-readable string.  Whenever a system call fails these classes also set
`ft_errno`, allowing calling code to react uniformly across platforms.

Additional helpers simplify common client/server patterns such as connecting to
a remote host or polling multiple sockets.  The abstractions hide the
differences between Unix-like systems and Windows winsock so the same code can
run everywhere.

### Template – Container Utilities

Templates similar to those in the C++ standard library are provided
here.  Highlights include `ft_sharedptr` and `ft_uniqueptr` smart
pointers, a `ft_vector` dynamic array and associative containers like
`ft_map` and `ft_unord_map`.

Iterator support and a small set of algorithms make these containers familiar
to anyone used to the STL.  They are designed to be header-only so they can be
included without separate compilation steps.

Every container stores an error flag that mirrors the global `ft_errno`.  When a
memory allocation fails or an index is out of range the object records the
appropriate code from `errno.hpp`.  Functions such as `ft_vector::getError()`,
`ft_map::getError()` and the `hasError`/`errorMessage` helpers on the smart
pointers allow applications to inspect these failures without resorting to
exceptions.  A default value is returned when an operation cannot be completed,
leaving the object in a valid but errored state.

### RNG – Random Utilities

Contains a pseudo random number helper `ft_dice_roll(number, faces)` as
well as a simple playing card `deck` container.

The routines use a small internal state to produce deterministic sequences
unless seeded differently, which is handy for tests or reproducible gameplay
mechanics.

### HTML – HTML Parsing

The `HTML` folder provides a tiny HTML parser capable of constructing a
tree of `html_node` objects.  Utility functions allow searching nodes,
serialising them to strings or files and managing attributes.

It is suitable for simple configuration formats or small embedded web tools and
is not intended to fully validate or sanitize complex documents.

### Encryption

`encryption` implements a basic XOR based scheme for saving small pieces
of data.  Functions include `be_saveGame`, `be_DecryptData` and
`be_getEncryptionKey`.

The algorithms are intentionally simple and offer only lightweight
obfuscation.  They can deter casual tampering with configuration or save files
but should not be used for serious security needs.

### File

This directory hosts portable wrappers for directory iteration and
creation.  `ft_opendir`, `ft_readdir` and `ft_closedir` mimic the POSIX
API but work on both Linux and Windows.

An additional `ft_mkdir` helper creates directories with appropriate
permissions on each system.  Together these functions form a small abstraction
layer for basic file system manipulation.

Other directories such as `Linux` and `Windows` hold platform specific
helpers used internally by some of the modules above.
