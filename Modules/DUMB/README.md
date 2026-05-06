# DUMB

`DUMB` stands for `Direct Unaccelerated Memory Blitter`.

This submodule is a small, low-level utility layer for simple interactive programs. It gives you:

- a software-rendered window surface through `ft_render_window`
- a minimal audio abstraction through `ft_sound_device` and `ft_sound_clip`
- simple polling-based controls through `ft_dumb_controls_*`
- small file helpers through `ft_read_file` and `ft_write_file`

At the repository level, `DUMB` is treated as a lightweight/legacy sandbox-style module. In practice it is still usable, and the demo program in `Demo/` shows the intended way to drive it.

## Public headers

- `render_window.hpp`
  Provides a window, framebuffer access, event polling, presenting, fullscreen toggling, and software pixel writes.
- `sound_device.hpp`
  Declares the base audio device API and the `ft_create_sound_device()` factory that chooses the platform backend.
- `sound_clip.hpp`
  Loads WAV data and exposes the decoded byte buffer plus its `ft_sound_spec`.
- `controls.hpp`
  Exposes a small fixed input set: up, down, left, right, confirm, and back.
- `dumb_io.hpp`
  Exposes `ft_read_file(...)` and `ft_write_file(...)`.

## Lifecycle

The main classes in this module follow the project lifecycle contract:

1. Construct the object.
2. Call `initialize(...)` before first real use.
3. Use the object.
4. Call `destroy()` when you are done.

Important notes:

- `ft_render_window`, `ft_sound_device`, and `ft_sound_clip` are not ready for use immediately after construction.
- Calling most operations before `initialize(...)` is lifecycle misuse.
- Destructors try to clean up, but normal code should still call `destroy()` explicitly.
- Optional thread safety is available through `enable_thread_safety()` / `disable_thread_safety()` on the lifecycle classes.

## Rendering flow

Typical render usage looks like this:

```cpp
#include "../DUMB/render_window.hpp"

int32_t run_window(void)
{
    ft_render_window window;
    ft_render_window_desc desc;
    ft_render_framebuffer *framebuffer;
    int32_t error_code;

    error_code = window.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    desc.width = 1280;
    desc.height = 720;
    desc.title = "DUMB window";
    desc.flags = FT_RENDER_WINDOW_FLAG_RESIZABLE;
    error_code = window.initialize(desc);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)window.destroy();
        return (error_code);
    }
    while (window.should_close() == FT_FALSE)
    {
        error_code = window.poll_events();
        if (error_code != FT_ERR_SUCCESS)
            break ;
        error_code = window.clear(0x101820U);
        if (error_code != FT_ERR_SUCCESS)
            break ;
        framebuffer = &window.framebuffer();
        if (framebuffer->width > 10 && framebuffer->height > 10)
            framebuffer->pixels[10 * framebuffer->width + 10] = 0xFFFFFFFFU;
        error_code = window.present();
        if (error_code != FT_ERR_SUCCESS)
            break ;
    }
    (void)window.destroy();
    return (error_code);
}
```

Notes:

- Rendering is software-based. You write pixels into the framebuffer and call `present()`.
- `poll_events()` updates window state, including the close request consumed by `should_close()`.
- `clear(...)`, `put_pixel(...)`, `framebuffer()`, and `depth_buffer()` are the core drawing entry points.

## Input flow

Input is polling-based and intentionally small:

```cpp
ft_dumb_controls_poll();
if (ft_dumb_control_is_down(FT_DUMB_CONTROL_LEFT) == FT_TRUE)
{
    /* move left */
}
if (ft_dumb_control_was_pressed(FT_DUMB_CONTROL_CONFIRM) == FT_TRUE)
{
    /* one-shot confirm action */
}
```

The current control set is:

- `FT_DUMB_CONTROL_UP`
- `FT_DUMB_CONTROL_DOWN`
- `FT_DUMB_CONTROL_LEFT`
- `FT_DUMB_CONTROL_RIGHT`
- `FT_DUMB_CONTROL_CONFIRM`
- `FT_DUMB_CONTROL_BACK`

## Audio flow

Audio is split into a device and clips:

```cpp
#include "../DUMB/sound_device.hpp"
#include "../DUMB/sound_clip.hpp"

int32_t play_clip_example(void)
{
    ft_sound_device *device;
    ft_sound_clip clip;
    ft_sound_spec spec;
    int32_t error_code;

    device = ft_create_sound_device();
    if (device == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    error_code = device->initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        delete device;
        return (error_code);
    }
    error_code = clip.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)device->destroy();
        delete device;
        return (error_code);
    }
    error_code = clip.load_wav("example.wav");
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)clip.destroy();
        (void)device->destroy();
        delete device;
        return (error_code);
    }
    spec = *clip.get_spec();
    error_code = device->open(&spec);
    if (error_code == FT_ERR_SUCCESS)
    {
        device->play(&clip);
        device->pause(0);
    }
    device->close();
    (void)clip.destroy();
    (void)device->destroy();
    delete device;
    return (error_code);
}
```

Notes:

- `ft_create_sound_device()` returns the backend implementation for the current platform.
- `ft_sound_clip::load_wav(...)` is the clip-loading entry point exposed by this module.
- `open(...)`, `play(...)`, `pause(...)`, `stop(...)`, and `close()` are implemented by the selected backend.

## File helpers

`dumb_io.hpp` exposes two small helpers:

- `ft_read_file(const char *path, char **out_buffer, ft_size_t *out_size)`
- `ft_write_file(const char *path, const char *buffer, ft_size_t size)`

Use them for simple whole-file reads and writes when you do not need a heavier stream abstraction.

## Platform notes

- Windowing, controls, and audio backend details are delegated into the `Compatebility/` module.
- The render path is intentionally unaccelerated and aimed at straightforward framebuffer-driven programs.
- The sound backends are thin adapters behind the common `ft_sound_device` interface.

## Related code

- `Demo/`
  Contains the most complete example of how this module is used in practice.
- `Compatebility/Compatebility_dumb_*`
  Contains the platform-specific helpers used by this module.
