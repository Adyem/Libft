#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

#include "../DUMB/dumb_render_internal.hpp"
#include <stdlib.h>

struct ft_render_macos_state
{
    NSWindow        *window;

    int             width;
    int             height;

    uint32_t        *pixels;

    bool            is_fullscreen;
    bool            should_close;
};

ft_render_platform_result ft_render_platform_get_primary_screen_size(ft_render_screen_size *out_size)
{
    NSScreen *screen;
    NSRect   frame;

    if (out_size == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    screen = [NSScreen mainScreen];
    if (screen == nil)
    {
        out_size->width = 0;
        out_size->height = 0;
        return ((ft_render_platform_result){ ft_render_error_platform_failure, 0 });
    }

    frame = [screen frame];

    out_size->width = (int)frame.size.width;
    out_size->height = (int)frame.size.height;

    if (out_size->width <= 0 || out_size->height <= 0)
    {
        return ((ft_render_platform_result){ ft_render_error_platform_failure, 0 });
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_create_window(
    void **out_platform_state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    ft_render_macos_state *state;
    NSRect rect;

    if (out_platform_state == NULL || out_framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_macos_state *)calloc(1, sizeof(ft_render_macos_state));
    if (state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_out_of_memory, 0 });
    }

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    state->width = desc.width;
    state->height = desc.height;
    state->is_fullscreen = false;
    state->should_close = false;

    state->pixels = (uint32_t *)malloc((size_t)(state->width * state->height * 4));
    if (state->pixels == NULL)
    {
        free(state);
        return ((ft_render_platform_result){ ft_render_error_out_of_memory, 0 });
    }

    rect = NSMakeRect(100, 100, desc.width, desc.height);
    state->window = [[NSWindow alloc]
        initWithContentRect:rect
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
        backing:NSBackingStoreBuffered
        defer:NO
    ];

    if (state->window == nil)
    {
        free(state->pixels);
        free(state);
        return ((ft_render_platform_result){ ft_render_error_platform_failure, 0 });
    }

    [state->window setTitle:[NSString stringWithUTF8String:desc.title]];
    [state->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = state->pixels;

    *out_platform_state = state;
    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_macos_state *state;

    if (platform_state == NULL || *platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_ok, 0 });
    }

    state = (ft_render_macos_state *)(*platform_state);

    if (state->window != nil)
    {
        [state->window close];
        state->window = nil;
    }

    if (state->pixels != NULL)
    {
        free(state->pixels);
        state->pixels = NULL;
    }

    free(state);
    *platform_state = NULL;

    if (framebuffer != NULL)
    {
        framebuffer->width = 0;
        framebuffer->height = 0;
        framebuffer->pixels = NULL;
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    bool *out_should_close
)
{
    ft_render_macos_state *state;
    NSEvent *event;

    if (platform_state == NULL || out_should_close == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    event = [NSApp nextEventMatchingMask:NSEventMaskAny
        untilDate:[NSDate distantPast]
        inMode:NSDefaultRunLoopMode
        dequeue:YES
    ];

    while (event != nil)
    {
        if ([event type] == NSEventTypeApplicationDefined)
        {
            /* nothing */
        }
        [NSApp sendEvent:event];
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
            untilDate:[NSDate distantPast]
            inMode:NSDefaultRunLoopMode
            dequeue:YES
        ];
    }

    *out_should_close = state->should_close;
    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_macos_state *state;
    CGColorSpaceRef color_space;
    CGContextRef context;
    CGImageRef image;
    size_t bytes_per_row;

    if (platform_state == NULL || framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    bytes_per_row = (size_t)(framebuffer->width * 4);
    color_space = CGColorSpaceCreateDeviceRGB();

    context = CGBitmapContextCreate(
        framebuffer->pixels,
        (size_t)framebuffer->width,
        (size_t)framebuffer->height,
        8,
        bytes_per_row,
        color_space,
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big
    );

    if (context == NULL)
    {
        CGColorSpaceRelease(color_space);
        return ((ft_render_platform_result){ ft_render_error_platform_failure, 0 });
    }

    image = CGBitmapContextCreateImage(context);

    NSView *view = [state->window contentView];
    [view lockFocus];
    CGContextRef ns_context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    CGRect rect = CGRectMake(0, 0, framebuffer->width, framebuffer->height);
    CGContextDrawImage(ns_context, rect, image);
    [view unlockFocus];

    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(color_space);

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    bool enabled
)
{
    ft_render_macos_state *state;

    if (platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    if (enabled == true && state->is_fullscreen == false)
    {
        [state->window toggleFullScreen:nil];
        state->is_fullscreen = true;
    }
    else if (enabled == false && state->is_fullscreen == true)
    {
        [state->window toggleFullScreen:nil];
        state->is_fullscreen = false;
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

#endif
