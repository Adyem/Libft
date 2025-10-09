# Bazel quick start

The Bazel template wires Libft into a workspace using a `cc_library` target.
It assumes the repository is checked out under `external/libft` relative to the
workspace root.

## Layout

```
my_app/
├── WORKSPACE
├── BUILD.bazel
├── src/
│   └── main.cpp
└── external/
    └── libft/
```

## Usage

1. Copy `WORKSPACE`, `BUILD.bazel`, and the `src` directory into a new workspace.
2. Adjust the `libft_root` repository path inside `BUILD.bazel` if Libft lives
   elsewhere.
3. Build and run the example:

   ```bash
   bazel run //:quick_start
   ```

The `cc_library` rule globs Libft sources while filtering out tests and tooling.
Link additional executables against the `@libft//:core` target to reuse the
library in your own binaries.
