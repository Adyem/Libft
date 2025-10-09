# CMake quick start

This template demonstrates a minimal CMake project that venders Libft as a
Git submodule (or any local checkout) and builds an application that prints the
length of a string with `ft_strlen`.

## Layout

```
my_app/
├── CMakeLists.txt          # Copied from this template
├── src/
│   └── main.cpp            # Sample program
└── external/
    └── libft/              # Libft checkout (this repository)
```

## Usage

1. Copy the `CMakeLists.txt` file and the `src` directory into your project
   root.
2. Clone Libft next to your sources (or adjust the `LIBFT_ROOT` cache variable
   in `CMakeLists.txt` to point to an existing checkout).
3. Configure and build the project:

   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/bin/quick_start
   ```

The CMake script gathers Libft translation units with `file(GLOB_RECURSE ...)`
and exports an `libft::core` target that links your executable.  You can replace
`src/main.cpp` with your own entry point and expand the target list as needed.
