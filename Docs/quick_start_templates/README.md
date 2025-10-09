# Quick-start templates

The samples in this directory show how to consume Libft from common build systems.
Each template is self-contained and expects the Libft repository to live next to
the generated project directory (for example, cloning Libft into `external/libft`
and using the template as your application root).  The build files gather the
necessary `.cpp` translation units from Libft, expose include paths, and link a
simple executable that prints the length of a string using `ft_strlen`.

## Template overview

- [`cmake/`](cmake/README.md): Demonstrates a modern CMake project that vendored
  Libft through `add_subdirectory` and reuses the manifest that powers the
  umbrella `FullLibft.hpp` header.
- [`meson/`](meson/README.md): Provides a Meson build that mirrors the CMake
  logic with a `static_library` target exported to the consuming executable.
- [`bazel/`](bazel/README.md): Shows how to wire Libft into a Bazel workspace
  using a `cc_library` target that aggregates the repository sources.

Each quick-start guide explains how to copy the template, customize the Libft
path, and build the resulting sample.  The templates are intentionally small so
you can adapt them to larger projects without sorting through unrelated files.
