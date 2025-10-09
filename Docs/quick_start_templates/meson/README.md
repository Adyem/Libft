# Meson quick start

The Meson template mirrors the CMake setup and is convenient when you prefer a
Pythonic configuration language.

## Layout

```
my_app/
├── meson.build
├── src/
│   └── main.cpp
└── external/
    └── libft/
```

## Usage

1. Copy `meson.build` and the `src` directory into a new project folder.
2. Adjust the `libft_root` variable in `meson.build` if your checkout lives in a
different location.
3. Configure and compile the sample:

   ```bash
   meson setup build
   meson compile -C build
   ./build/quick_start
   ```

Meson creates a static library containing the Libft sources and links it into a
simple executable.  Replace `src/main.cpp` with your application entry point and
reuse the `libft_lib` target wherever you need Libft functionality.
