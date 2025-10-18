# Code generation scaffolds

The `tools/generate_scaffold.py` helper bootstraps boilerplate that matches the
project's coding conventions. It removes the repetitive setup work when adding
new modules or tests while keeping private error tracking, Allman braces, and
`FT_TEST` usage consistent.

## Requirements

The script only depends on the Python 3 standard library. Run it directly from
the repository root:

```bash
./tools/generate_scaffold.py --help
```

## Generating module scaffolds

Use the `module` subcommand to create paired header and source files. Pass the
snake_case class name and an optional destination directory. The script preserves
existing files unless `--force` is specified.

```bash
./tools/generate_scaffold.py module cma_example_guard --directory CMA
```

The command above produces `CMA/cma_example_guard.hpp` and
`CMA/cma_example_guard.cpp` with the following features:

- Private `_error_code` member with `set_error` to propagate `ft_errno` updates.
- Constructors and destructors that follow the "`return ;`" convention.
- Getter helpers returning the stored error code and `ft_strerror` message.

After generation, fill in the class-specific logic while keeping the provided
structure intact.

## Generating regression test scaffolds

The `test` subcommand writes an `FT_TEST` skeleton into the target directory.
Specify a short suffix for the filename and, optionally, the test identifier and
description.

```bash
./tools/generate_scaffold.py test cma_example --test-name test_cma_example_guard --description "describe behaviour"
```

The example generates `Test/Test/test_cma_example.cpp` with the harness includes
and a passing assertion placeholder.

## Updating existing files

Both subcommands skip files that already exist. Supply `--force` to overwrite
them:

```bash
./tools/generate_scaffold.py module cma_example_guard --directory CMA --force
```

Use this flag cautiously because it replaces the entire file content.
