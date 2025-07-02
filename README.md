# Full Libft

Full Libft is a collection of C++ libraries that provide many utilities used across
multiple projects.  It bundles reimplementations of C standard functions,
custom memory management, formatted output, networking helpers and more.  Each
feature lives inside its own directory and can be built on its own or combined
into a single static library.

## Building

Run `make` in the repository root to create `Full_Libft.a`.  Use `make debug`
for a debug build, `make both` to build both variants, and `make re` to clean
and rebuild everything.

```bash
$ make       # build release library
$ make debug # build debug library
```

## Tests

A simple test suite resides in the `Test` folder.  Build and run it with:

```bash
$ make -C Test
$ ./Test/libft_tests
```

## Using the library

Link the produced `Full_Libft.a` (or the debug version) against your C++
project and include the headers from the modules you need.  Individual
subdirectories also provide Makefiles if you only want a specific component.

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

### Libft – C Utility Reimplementations

`Libft` contains lightweight replacements for many common C library
functions.  Examples include `ft_strlen`, `ft_atoi`, `ft_isalpha`,
memory operations such as `ft_memcpy` and string search helpers like
`ft_strchr`.  These functions mimic the behaviour of their standard
counterparts while conforming to the project coding style.

### Printf – Formatted Output

The `Printf` folder implements a small `pf_printf` function compatible
with the `printf` format string syntax.  It supports the most common
format specifiers and also provides `pf_printf_fd` for writing to an
arbitrary file descriptor.

### ReadLine – Interactive Input

`ReadLine` offers a simple line editor for terminal applications.  The
public API consists of:

- `rl_readline(prompt)` – read a line from standard input while handling
  arrow keys and history.
- `rl_add_suggestion(word)` and `rl_clear_suggestions()` – manage tab
  completion entries.
- `rl_clear_history()` – erase the stored input history.

### PThread – Threading Helpers

This module wraps a few POSIX thread primitives.  Functions such as
`pt_thread_create` and `pt_thread_join` mirror `pthread_create` and
`pthread_join`.  A lightweight `pt_mutex` class implements a mutex with
try-lock and error reporting features.

### CPP_class – Helper Classes

`CPP_class` provides some small utility classes used across the
library:

- `ft_file` – RAII wrapper around a file descriptor with methods for
  reading, writing and formatted output.
- `ft_string` – a dynamically sized string class supporting appending and
  direct C-string access.
- `data_buffer` – a simple growable buffer for binary data.

### Errno – Error Handling

Defines the `ft_errno` variable and a large set of library specific
error codes.  Use `ft_strerror` to translate an error code into a
message and `ft_perror` to print a formatted error string.

### Networking – Cross Platform Sockets

This module contains wrappers for basic socket operations along with two
helper classes:

- `SocketConfig` – stores configuration used when creating sockets.
- `ft_socket` – RAII socket wrapper supporting client/server setup,
  non-blocking mode, broadcasting and more.  Helper functions such as
  `nw_bind`, `nw_listen` and `nw_accept` directly expose the underlying
  system calls.

### Template – Container Utilities

Templates similar to those in the C++ standard library are provided
here.  Highlights include `ft_sharedptr` and `ft_uniqueptr` smart
pointers, a `ft_vector` dynamic array and associative containers like
`ft_map` and `ft_unord_map`.

### RNG – Random Utilities

Contains a pseudo random number helper `ft_dice_roll(number, faces)` as
well as a simple playing card `deck` container.

### HTML – HTML Parsing

The `HTML` folder provides a tiny HTML parser capable of constructing a
tree of `html_node` objects.  Utility functions allow searching nodes,
serialising them to strings or files and managing attributes.

### Encryption

`encryption` implements a basic XOR based scheme for saving small pieces
of data.  Functions include `be_saveGame`, `be_DecryptData` and
`be_getEncryptionKey`.

### File

This directory hosts portable wrappers for directory iteration and
creation.  `ft_opendir`, `ft_readdir` and `ft_closedir` mimic the POSIX
API but work on both Linux and Windows.

Other directories such as `Linux` and `Windows` hold platform specific
helpers used internally by some of the modules above.
