#if defined(__APPLE__)

#import <ApplicationServices/ApplicationServices.h>

#include "compatebility_dumb_controls_internal.hpp"

static ft_bool dumb_controls_macos_key_is_down(CGKeyCode key_code_primary,
    CGKeyCode key_code_secondary, ft_bool use_secondary)
{
    if (CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState,
        key_code_primary) != 0)
    {
        return (FT_TRUE);
    }
    if (use_secondary == FT_TRUE
        && CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState,
        key_code_secondary) != 0)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    if (control == FT_DUMB_CONTROL_UP)
        return (dumb_controls_macos_key_is_down(13U, 126U, FT_TRUE));
    if (control == FT_DUMB_CONTROL_DOWN)
        return (dumb_controls_macos_key_is_down(1U, 125U, FT_TRUE));
    if (control == FT_DUMB_CONTROL_LEFT)
        return (dumb_controls_macos_key_is_down(0U, 123U, FT_TRUE));
    if (control == FT_DUMB_CONTROL_RIGHT)
        return (dumb_controls_macos_key_is_down(2U, 124U, FT_TRUE));
    if (control == FT_DUMB_CONTROL_CONFIRM)
        return (dumb_controls_macos_key_is_down(36U, 49U, FT_TRUE));
    if (control == FT_DUMB_CONTROL_BACK)
        return (dumb_controls_macos_key_is_down(53U, 0U, FT_FALSE));
    return (FT_FALSE);
}

ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void)
{
    static ft_bool initialized = FT_FALSE;
    ft_dumb_mouse_delta delta;
    CGEventRef event;
    CGPoint position;
    CGPoint center;
    CGDirectDisplayID display_id;

    delta.x = 0;
    delta.y = 0;
    event = CGEventCreate(NULL);
    if (event == NULL)
    {
        initialized = FT_FALSE;
        return (delta);
    }
    position = CGEventGetLocation(event);
    CFRelease(event);
    display_id = CGMainDisplayID();
    center.x = static_cast<CGFloat>(CGDisplayPixelsWide(display_id)) * 0.5;
    center.y = static_cast<CGFloat>(CGDisplayPixelsHigh(display_id)) * 0.5;
    if (initialized == FT_TRUE)
    {
        delta.x = static_cast<int32_t>(position.x - center.x);
        delta.y = static_cast<int32_t>(position.y - center.y);
    }
    CGWarpMouseCursorPosition(center);
    CGAssociateMouseAndMouseCursorPosition(true);
    initialized = FT_TRUE;
    return (delta);
}

#endif
