# GPGR

`GPGR` is the OpenGL-backed window and shader module used by the renderer demo path. It provides a small window abstraction plus a shader wrapper and the platform-specific GL function loader behind them.

## Public Types

- `ft_gpu_window` - Abstract window interface for creating a GL-capable window, polling events, swapping buffers, and reading input state.
- `ft_gpu_shader` - Lifecycle shader wrapper for compiling, binding, querying uniforms, moving ownership, and releasing a linked GL program.

## Public API

- `ft_gpu_window::create()` - Allocates the platform-specific window implementation.
- `ft_gpu_window::initialize(...)` - Creates a window and initializes the GL context.
- `ft_gpu_window::destroy()` - Releases the window, context, and platform resources and returns `FT_ERR_*`.
- `ft_gpu_window::move(...)` - Transfers the platform window state from another window implementation.
- `ft_gpu_window::poll_events()` - Pumps native events and updates input state.
- `ft_gpu_window::swap_buffers()` - Presents the current frame.
- `ft_gpu_window::should_close()` - Reports whether the window should exit.
- `ft_gpu_window::get_width()` / `get_height()` - Return the current drawable size.
- `ft_gpu_window::get_mouse_x()` / `get_mouse_y()` - Return the latest mouse coordinates.
- `ft_gpu_window::was_mouse_clicked()` - Reports whether the primary mouse button was clicked since the last poll.
- `ft_gpu_window::set_cursor_visible(...)` - Toggles cursor visibility.
- `ft_gpu_window::was_settings_key_pressed()` - Reports whether the module-specific settings key was pressed.

- `ft_gpu_shader::initialize(...)` - Compiles and links a vertex/fragment shader pair.
- `ft_gpu_shader::destroy()` - Deletes the linked program and returns `FT_ERR_*`.
- `ft_gpu_shader::move(...)` - Transfers a linked program from another shader wrapper.
- `ft_gpu_shader::use()` - Makes the program current.
- `ft_gpu_shader::uniform(...)` - Returns a uniform location.
- `ft_gpu_shader::ready()` - Reports whether a linked program exists.

## Loader

- `gpgr_load_gl_functions()` - Loads the GL entry points required by the backend platform implementations.
