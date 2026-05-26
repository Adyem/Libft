#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>

#include "../DUMB/dumb_render_internal.hpp"
#include <stdlib.h>

struct ft_render_macos_state
{
    NSWindow        *window;
    NSObject        *window_delegate;
    NSRect          windowed_frame;
    NSUInteger      windowed_style_mask;

    int32_t         width;
    int32_t         height;

    uint32_t        *pixels;
    uint32_t        *display_pixels;

    ft_bool         is_fullscreen;
    ft_bool         should_close;
};

static uint32_t ft_render_macos_to_bgra(uint32_t source_color)
{
    return (0xFF000000U | (source_color & 0x00FFFFFFU));
}

@interface FtRenderMacOSWindowDelegate : NSObject <NSWindowDelegate>
{
    ft_render_macos_state *_state;
}
- (id)initWithRenderState:(ft_render_macos_state *)state;
- (BOOL)windowShouldClose:(id)sender;
@end

@implementation FtRenderMacOSWindowDelegate
- (id)initWithRenderState:(ft_render_macos_state *)state
{
    self = [super init];
    if (self != nil)
    {
        _state = state;
    }
    return (self);
}

- (BOOL)windowShouldClose:(id)sender
{
    (void)sender;
    if (_state != NULL)
    {
        _state->should_close = FT_TRUE;
    }
    return (NO);
}
@end

ft_render_platform_result ft_render_platform_get_primary_screen_size(ft_render_screen_size *out_size)
{
    NSScreen *screen;
    NSRect   frame;

    if (out_size == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }

    screen = [NSScreen mainScreen];
    if (screen == nil)
    {
        out_size->width = 0;
        out_size->height = 0;
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }

    frame = [screen frame];

    out_size->width = static_cast<int32_t>(frame.size.width);
    out_size->height = static_cast<int32_t>(frame.size.height);

    if (out_size->width <= 0 || out_size->height <= 0)
    {
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }

    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
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
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }

    state = (ft_render_macos_state *)calloc(1, sizeof(ft_render_macos_state));
    if (state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
    }

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    state->width = desc.width;
    state->height = desc.height;
    state->is_fullscreen = FT_FALSE;
    state->should_close = FT_FALSE;

    state->pixels = (uint32_t *)malloc((size_t)(state->width * state->height * 4));
    if (state->pixels == NULL)
    {
        free(state);
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
    }
    state->display_pixels = (uint32_t *)malloc((size_t)(
        state->width * state->height * 4));
    if (state->display_pixels == NULL)
    {
        free(state->pixels);
        free(state);
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
    }

    rect = NSMakeRect(100, 100, desc.width, desc.height);
    state->windowed_frame = rect;
    state->windowed_style_mask = (NSWindowStyleMaskTitled
        | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable);
    state->window = [[NSWindow alloc]
        initWithContentRect:rect
        styleMask:state->windowed_style_mask
        backing:NSBackingStoreBuffered
        defer:NO
    ];

    if (state->window == nil)
    {
        free(state->display_pixels);
        free(state->pixels);
        free(state);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }

    state->window_delegate = [[FtRenderMacOSWindowDelegate alloc]
        initWithRenderState:state];
    if (state->window_delegate == nil)
    {
        [state->window close];
        free(state->display_pixels);
        free(state->pixels);
        free(state);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }
    [state->window setDelegate:(id<NSWindowDelegate>)state->window_delegate];
    [state->window setTitle:[NSString stringWithUTF8String:desc.title]];
    [[state->window contentView] setWantsLayer:YES];
    [[[state->window contentView] layer] setContentsGravity:kCAGravityResize];
    [state->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    if ((desc.flags & FT_RENDER_WINDOW_FLAG_FULLSCREEN) != 0)
    {
        (void)ft_render_platform_set_fullscreen(state, FT_TRUE);
    }

    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = state->pixels;

    *out_platform_state = state;
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_macos_state *state;

    if (platform_state == NULL || *platform_state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
    }

    state = (ft_render_macos_state *)(*platform_state);

    if (state->window != nil)
    {
        [state->window setDelegate:nil];
        [state->window close];
        state->window = nil;
    }
    if (state->window_delegate != nil)
    {
        [state->window_delegate release];
        state->window_delegate = nil;
    }

    if (state->pixels != NULL)
    {
        free(state->pixels);
        state->pixels = NULL;
    }
    if (state->display_pixels != NULL)
    {
        free(state->display_pixels);
        state->display_pixels = NULL;
    }

    free(state);
    *platform_state = NULL;

    if (framebuffer != NULL)
    {
        framebuffer->width = 0;
        framebuffer->height = 0;
        framebuffer->pixels = NULL;
    }

    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    ft_bool *out_should_close
)
{
    ft_render_macos_state *state;
    NSEvent *event;

    if (platform_state == NULL || out_should_close == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    event = [NSApp nextEventMatchingMask:NSEventMaskAny
        untilDate:[NSDate distantPast]
        inMode:NSDefaultRunLoopMode
        dequeue:YES
    ];

    while (event != nil)
    {
        if ([event type] == NSEventTypeKeyDown
            || [event type] == NSEventTypeKeyUp
            || [event type] == NSEventTypeFlagsChanged)
        {
            /* Controls are polled through CGEventSourceKeyState. */
        }
        else if ([event type] == NSEventTypeApplicationDefined)
        {
            /* nothing */
        }
        else
        {
            [NSApp sendEvent:event];
        }
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
            untilDate:[NSDate distantPast]
            inMode:NSDefaultRunLoopMode
            dequeue:YES
        ];
    }

    *out_should_close = state->should_close;
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer,
    const ft_render_depth_buffer *depth_buffer
)
{
    ft_render_macos_state *state;
    CGColorSpaceRef color_space;
    CGContextRef context;
    CGImageRef image;
    NSView *view;
    size_t bytes_per_row;
    size_t pixel_count;
    size_t index;

    (void)depth_buffer;

    if (platform_state == NULL || framebuffer == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    if (framebuffer->pixels == NULL
        || framebuffer->width != state->width
        || framebuffer->height != state->height
        || state->display_pixels == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_STATE, 0 });
    }
    pixel_count = (size_t)framebuffer->width * (size_t)framebuffer->height;
    index = 0;
    while (index < pixel_count)
    {
        state->display_pixels[index] = ft_render_macos_to_bgra(
            framebuffer->pixels[index]);
        index = index + 1;
    }
    bytes_per_row = (size_t)(framebuffer->width * 4);
    color_space = CGColorSpaceCreateDeviceRGB();

    context = CGBitmapContextCreate(
        state->display_pixels,
        (size_t)framebuffer->width,
        (size_t)framebuffer->height,
        8,
        bytes_per_row,
        color_space,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little
    );

    if (context == NULL)
    {
        CGColorSpaceRelease(color_space);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }

    image = CGBitmapContextCreateImage(context);
    view = [state->window contentView];
    if (view == nil || [view layer] == nil)
    {
        CGImageRelease(image);
        CGContextRelease(context);
        CGColorSpaceRelease(color_space);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }
    [[view layer] setContents:(id)image];

    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(color_space);

    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    ft_bool enabled
)
{
    ft_render_macos_state *state;

    if (platform_state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }

    state = (ft_render_macos_state *)platform_state;

    if (enabled == FT_TRUE && state->is_fullscreen == FT_FALSE)
    {
        NSScreen *screen;
        NSRect screen_frame;

        screen = [state->window screen];
        if (screen == nil)
        {
            screen = [NSScreen mainScreen];
        }
        if (screen == nil)
        {
            return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
        }
        state->windowed_frame = [state->window frame];
        state->windowed_style_mask = [state->window styleMask];
        screen_frame = [screen frame];
        [state->window setStyleMask:NSWindowStyleMaskBorderless];
        [state->window setFrame:screen_frame display:YES animate:NO];
        [state->window setLevel:NSMainMenuWindowLevel + 1];
        [state->window makeKeyAndOrderFront:nil];
        state->is_fullscreen = FT_TRUE;
    }
    else if (enabled == FT_FALSE && state->is_fullscreen == FT_TRUE)
    {
        [state->window setLevel:NSNormalWindowLevel];
        [state->window setStyleMask:state->windowed_style_mask];
        [state->window setFrame:state->windowed_frame display:YES animate:NO];
        [state->window makeKeyAndOrderFront:nil];
        state->is_fullscreen = FT_FALSE;
    }

    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

#endif
