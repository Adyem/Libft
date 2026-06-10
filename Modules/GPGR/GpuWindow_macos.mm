#if defined(__APPLE__)

#include "GpuWindow_macos.hpp"

@interface GpuWindowDelegate : NSObject<NSWindowDelegate>
{
    bool *_close_flag;
}
- (instancetype)initWithFlag:(bool *)flag;
@end

@implementation GpuWindowDelegate
- (instancetype)initWithFlag:(bool *)flag
{
    self = [super init];
    if (self != nil)
        _close_flag = flag;
    return (self);
}

- (BOOL)windowShouldClose:(id)sender
{
    (void)sender;
    if (_close_flag != nullptr)
        *_close_flag = true;
    return (NO);
}
@end

@interface GpuOpenGLView : NSOpenGLView
@end

@implementation GpuOpenGLView
- (BOOL)acceptsFirstResponder { return (YES); }
- (void)keyDown:(NSEvent *)event { (void)event; }
- (void)keyUp:(NSEvent *)event { (void)event; }
@end

struct GpuWindowMacOSState
{
    NSWindow         *window;
    GpuOpenGLView    *gl_view;
    NSOpenGLContext  *gl_context;
    GpuWindowDelegate *delegate;
    bool              close_flag;
    int               width;
    int               height;
    int               mouse_x;
    int               mouse_y;
    bool              mouse_clicked;
    bool              settings_key_pressed;
    bool              cursor_visible;
};

class GpuWindowMacOS : public GpuWindow
{
    GpuWindowMacOSState *_state;

    public:
        GpuWindowMacOS() noexcept : _state(nullptr) {}
        ~GpuWindowMacOS() override { destroy(); }

        bool initialize(const char *title, int width, int height,
            bool fullscreen) noexcept override;
        void destroy() noexcept override;
        void poll_events() noexcept override;
        void swap_buffers() noexcept override;
        bool should_close() const noexcept override;
        int  get_width()  const noexcept override;
        int  get_height() const noexcept override;
        int  get_mouse_x() const noexcept override;
        int  get_mouse_y() const noexcept override;
        bool was_mouse_clicked() const noexcept override;
        void set_cursor_visible(bool visible) noexcept override;
        bool was_settings_key_pressed() const noexcept override;
};

bool GpuWindowMacOS::initialize(const char *title, int width, int height,
    bool fullscreen) noexcept
{
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, static_cast<NSOpenGLPixelFormatAttribute>(
            NSOpenGLProfileVersion3_2Core),
        NSOpenGLPFAColorSize,   static_cast<NSOpenGLPixelFormatAttribute>(24),
        NSOpenGLPFADepthSize,   static_cast<NSOpenGLPixelFormatAttribute>(24),
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        static_cast<NSOpenGLPixelFormatAttribute>(0)
    };

    _state = new (std::nothrow) GpuWindowMacOSState();
    if (_state == nullptr)
        return (false);
    _state->close_flag           = false;
    _state->mouse_x              = 0;
    _state->mouse_y              = 0;
    _state->mouse_clicked        = false;
    _state->settings_key_pressed = false;
    _state->cursor_visible       = true;

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc]
        initWithAttributes:attrs];
    if (fmt == nil)
    {
        delete _state;
        _state = nullptr;
        return (false);
    }

    NSRect content_rect;
    NSUInteger style;
    if (fullscreen)
    {
        content_rect = [[NSScreen mainScreen] frame];
        style = NSWindowStyleMaskBorderless;
        _state->width  = static_cast<int>(content_rect.size.width);
        _state->height = static_cast<int>(content_rect.size.height);
    }
    else
    {
        content_rect = NSMakeRect(0, 0,
            static_cast<CGFloat>(width), static_cast<CGFloat>(height));
        style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
            | NSWindowStyleMaskMiniaturizable;
        _state->width  = width;
        _state->height = height;
    }

    _state->gl_view = [[GpuOpenGLView alloc]
        initWithFrame:content_rect pixelFormat:fmt];
    if (_state->gl_view == nil)
    {
        delete _state;
        _state = nullptr;
        return (false);
    }
    [_state->gl_view setWantsBestResolutionOpenGLSurface:NO];

    _state->window = [[NSWindow alloc]
        initWithContentRect:content_rect
        styleMask:style
        backing:NSBackingStoreBuffered
        defer:NO];
    if (_state->window == nil)
    {
        delete _state;
        _state = nullptr;
        return (false);
    }

    _state->delegate = [[GpuWindowDelegate alloc]
        initWithFlag:&_state->close_flag];
    [_state->window setDelegate:_state->delegate];
    [_state->window setTitle:[NSString stringWithUTF8String:title]];
    [_state->window setContentView:_state->gl_view];
    [_state->window makeKeyAndOrderFront:nil];
    [_state->window makeFirstResponder:_state->gl_view];
    [NSApp activateIgnoringOtherApps:YES];

    if (fullscreen)
        [_state->window setLevel:NSMainMenuWindowLevel + 1];

    _state->gl_context = [_state->gl_view openGLContext];
    [_state->gl_context makeCurrentContext];

    GLint swap = 1;
    [_state->gl_context setValues:&swap
        forParameter:NSOpenGLContextParameterSwapInterval];

    return (true);
}

void GpuWindowMacOS::destroy() noexcept
{
    if (_state == nullptr)
        return ;
    [NSOpenGLContext clearCurrentContext];
    if (_state->window != nil)
        [_state->window close];
    delete _state;
    _state = nullptr;
}

void GpuWindowMacOS::poll_events() noexcept
{
    if (_state == nullptr) return;
    _state->mouse_clicked        = false;
    _state->settings_key_pressed = false;
    @autoreleasepool
    {
        NSEvent *event;
        while ((event = [NSApp
                nextEventMatchingMask:NSEventMaskAny
                untilDate:nil
                inMode:NSDefaultRunLoopMode
                dequeue:YES]) != nil)
        {
            NSEventType t = [event type];
            if (t == NSEventTypeKeyDown)
            {
                if ([event keyCode] == 46U)
                    _state->settings_key_pressed = true;
                continue;
            }
            if (t == NSEventTypeKeyUp || t == NSEventTypeFlagsChanged)
                continue;
            if (t == NSEventTypeLeftMouseDown)
                _state->mouse_clicked = true;
            if (t == NSEventTypeMouseMoved || t == NSEventTypeLeftMouseDragged
                || t == NSEventTypeRightMouseDragged
                || t == NSEventTypeLeftMouseDown || t == NSEventTypeLeftMouseUp)
            {
                NSPoint loc = [event locationInWindow];
                if (_state->window != nil)
                {
                    NSRect fr = [[_state->window contentView] frame];
                    _state->mouse_x = static_cast<int>(loc.x);
                    _state->mouse_y = static_cast<int>(fr.size.height - loc.y);
                }
            }
            [NSApp sendEvent:event];
        }
    }
    if (_state->gl_context != nil)
        [_state->gl_context makeCurrentContext];
}

void GpuWindowMacOS::swap_buffers() noexcept
{
    if (_state != nullptr && _state->gl_context != nil)
        [_state->gl_context flushBuffer];
}

bool GpuWindowMacOS::should_close() const noexcept
{
    return (_state != nullptr && _state->close_flag);
}

int GpuWindowMacOS::get_width() const noexcept
{
    if (_state != nullptr)
        return (_state->width);
    return (0);
}

int GpuWindowMacOS::get_height() const noexcept
{
    if (_state != nullptr)
        return (_state->height);
    return (0);
}

int GpuWindowMacOS::get_mouse_x() const noexcept
{
    if (_state != nullptr)
        return (_state->mouse_x);
    return (0);
}

int GpuWindowMacOS::get_mouse_y() const noexcept
{
    if (_state != nullptr)
        return (_state->mouse_y);
    return (0);
}

bool GpuWindowMacOS::was_mouse_clicked() const noexcept
{
    return (_state != nullptr && _state->mouse_clicked);
}

void GpuWindowMacOS::set_cursor_visible(bool visible) noexcept
{
    if (_state == nullptr) return;
    if (visible && !_state->cursor_visible)
        [NSCursor unhide];
    else if (!visible && _state->cursor_visible)
        [NSCursor hide];
    _state->cursor_visible = visible;
}

bool GpuWindowMacOS::was_settings_key_pressed() const noexcept
{
    return (_state != nullptr && _state->settings_key_pressed);
}

GpuWindow *GpuWindow::create() noexcept
{
    return (new (std::nothrow) GpuWindowMacOS());
}

#endif
