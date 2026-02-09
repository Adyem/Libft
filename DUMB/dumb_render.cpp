#include "dumb_render_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"

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
    this->disable_thread_safety();
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_render_window::runtime_mutex(void)
{
    if (!this->_mutex)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

int ft_render_window::initialize(const ft_render_window_desc &desc)
{
    int                       lock_status;
    int                       unlock_status;
    ft_render_platform_result platform_result;

    lock_status = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_status != FT_ERR_SUCCESSS)
        return (lock_status);
    if (this->_is_initialized == true)
    {
        unlock_status = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESSS)
            return (unlock_status);
        ft_render_push_success();
        return (ft_render_ok);
    }
    if (desc.width <= 0 || desc.height <= 0 || desc.title == NULL)
    {
        ft_render_push_error(ft_render_error_invalid_argument, 0);
        unlock_status = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESSS)
            return (unlock_status);
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
        unlock_status = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESSS)
            return (unlock_status);
        return (platform_result.error_code);
    }
    this->_is_initialized = true;
    this->_should_close = false;
    unlock_status = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_status != FT_ERR_SUCCESSS)
        return (unlock_status);
    ft_render_push_success();
    return (ft_render_ok);
}

void ft_render_window::shutdown(void)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_is_initialized != false)
    {
        platform_result = ft_render_platform_destroy_window(
            &this->_platform_state,
            &this->_framebuffer
        );
        if (platform_result.error_code != ft_render_ok)
            ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        else
            ft_render_push_success();

        this->_platform_state = NULL;
        this->_framebuffer.width = 0;
        this->_framebuffer.height = 0;
        this->_framebuffer.pixels = NULL;

        this->_is_initialized = false;
        this->_should_close = true;
    }
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(unlock_error);
    return ;
}

int ft_render_window::poll_events(void)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_poll_events(
        this->_platform_state,
        &this->_should_close
    );
    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    ft_render_push_success();
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    return (ft_render_ok);
}

int ft_render_window::present(void)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_present(
        this->_platform_state,
        &this->_framebuffer
    );
    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    ft_render_push_success();
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    return (ft_render_ok);
}

ft_render_framebuffer &ft_render_window::framebuffer(void)
{
    return (this->_framebuffer);
}

int ft_render_window::clear(uint32_t color)
{
    int                          x;
    int                          y;
    int                          width;
    int                          height;
    uint32_t                     *pixels;
    int                          index;
    int                          lock_error;
    int                          unlock_error;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        return (lock_error);
    }
    if (this->_is_initialized == false)
    {
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        ft_render_push_error(ft_render_error_not_initialized, 0);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
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
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    ft_render_push_success();
    return (ft_render_ok);
}

int ft_render_window::put_pixel(int x, int y, uint32_t color)
{
    int lock_error;
    int unlock_error;
    int width;
    int height;
    int index;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    width = this->_framebuffer.width;
    height = this->_framebuffer.height;
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        ft_render_push_error(ft_render_error_invalid_argument, 0);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (ft_render_error_invalid_argument);
    }
    index = (y * width) + x;
    this->_framebuffer.pixels[index] = color;
    ft_render_push_success();
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    return (ft_render_ok);
}

int ft_render_window::set_fullscreen(bool enabled)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    lock_error = pt_recursive_mutex_lock_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        ft_render_push_error(ft_render_error_not_initialized, 0);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_set_fullscreen(
        this->_platform_state,
        enabled
    );
    if (platform_result.error_code != ft_render_ok)
    {
        ft_render_push_error(platform_result.error_code, platform_result.system_error_code);
        unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    ft_render_push_success();
    unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    return (ft_render_ok);
}

bool ft_render_window::should_close(void) const
{
    return (this->_should_close);
}

int ft_render_window::prepare_thread_safety(void) noexcept
{
    if (this->_mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_render_window::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_render_window::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void ft_render_window::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_render_window::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}
