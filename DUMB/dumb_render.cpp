#include "dumb_render_internal.hpp"
#include "../Errno/errno.hpp"


static void ft_render_push_success(void)
{
    ft_global_error_stack_push(ft_render_ok);
    return ;
}

static void ft_render_push_error(int error_code, int system_error_code)
{
    int mapped_error;

    mapped_error = error_code;
    if (error_code == ft_render_error_platform_failure)
    {
        mapped_error = ft_map_system_error(system_error_code);
    }
    ft_global_error_stack_push(mapped_error);
    return ;
}

ft_render_screen_size ft_render_get_primary_screen_size(void)
{
    ft_render_platform_result  platform_result;
    ft_render_screen_size      size;

    size.width = 0;
    size.height = 0;

    platform_result = ft_render_platform_get_primary_screen_size(&size);
    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        return (size);
    }

    ft_render_push_success();
    return (size);
}

ft_render_window::ft_render_window(void)
{
    this->_framebuffer.width = 0;
    this->_framebuffer.height = 0;
    this->_framebuffer.pixels = NULL;

    this->_is_initialized = false;
    this->_should_close = false;
    this->_platform_state = NULL;
    return ;
}

ft_render_window::~ft_render_window(void)
{
    this->shutdown();
    return ;
}

std::recursive_mutex &ft_render_window::runtime_mutex(void)
{
    return (this->_mutex);
}

int ft_render_window::initialize(const ft_render_window_desc &desc)
{
    ft_render_platform_result  platform_result;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == true)
    {
        ft_render_push_success();
        return (ft_render_ok);
    }
    if (desc.width <= 0 || desc.height <= 0 || desc.title == NULL)
    {
        ft_render_push_error(ft_render_error_invalid_argument, 0);
        return (ft_render_error_invalid_argument);
    }

    platform_result = ft_render_platform_create_window(
        &this->_platform_state,
        &this->_framebuffer,
        desc
    );

    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        return (platform_result.error_code);
    }

    this->_is_initialized = true;
    this->_should_close = false;

    ft_render_push_success();
    return (ft_render_ok);
}

void ft_render_window::shutdown(void)
{
    ft_render_platform_result  platform_result;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        return ;
    }

    platform_result = ft_render_platform_destroy_window(
        &this->_platform_state,
        &this->_framebuffer
    );
    (void)platform_result;

    this->_platform_state = NULL;
    this->_framebuffer.width = 0;
    this->_framebuffer.height = 0;
    this->_framebuffer.pixels = NULL;

    this->_is_initialized = false;
    this->_should_close = true;
    return ;
}

int ft_render_window::poll_events(void)
{
    ft_render_platform_result  platform_result;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        return (ft_render_error_not_initialized);
    }

    platform_result = ft_render_platform_poll_events(
        this->_platform_state,
        &this->_should_close
    );

    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        return (platform_result.error_code);
    }

    ft_render_push_success();
    return (ft_render_ok);
}

int ft_render_window::present(void)
{
    ft_render_platform_result  platform_result;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        return (ft_render_error_not_initialized);
    }

    platform_result = ft_render_platform_present(
        this->_platform_state,
        &this->_framebuffer
    );

    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        return (platform_result.error_code);
    }

    ft_render_push_success();
    return (ft_render_ok);
}

ft_render_framebuffer &ft_render_window::framebuffer(void)
{
    return (this->_framebuffer);
}

int ft_render_window::clear(uint32_t color)
{
    int         x;
    int         y;
    int         width;
    int         height;
    uint32_t    *pixels;
    int         index;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        return (ft_render_error_not_initialized);
    }

    width = this->_framebuffer.width;
    height = this->_framebuffer.height;
    pixels = this->_framebuffer.pixels;

    y = 0;
    while (y < height)
    {
        x = 0;
        while (x < width)
        {
            index = (y * width) + x;
            pixels[index] = color;
            x = x + 1;
        }
        y = y + 1;
    }

    ft_render_push_success();
    return (ft_render_ok);
}

int ft_render_window::put_pixel(int x, int y, uint32_t color)
{
    int width;
    int height;
    int index;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        return (ft_render_error_not_initialized);
    }

    width = this->_framebuffer.width;
    height = this->_framebuffer.height;

    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        ft_render_push_error(ft_render_error_invalid_argument, 0);
        return (ft_render_error_invalid_argument);
    }

    index = (y * width) + x;
    this->_framebuffer.pixels[index] = color;

    ft_render_push_success();
    return (ft_render_ok);
}

int ft_render_window::set_fullscreen(bool enabled)
{
    ft_render_platform_result  platform_result;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        return (ft_render_error_not_initialized);
    }

    platform_result = ft_render_platform_set_fullscreen(
        this->_platform_state,
        enabled
    );

    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        return (platform_result.error_code);
    }

    ft_render_push_success();
    return (ft_render_ok);
}

bool ft_render_window::should_close(void) const
{
    return (this->_should_close);
}

