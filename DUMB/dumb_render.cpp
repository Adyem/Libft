#include "dumb_render_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static int create_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    pt_recursive_mutex *created_mutex;
    int initialize_error;

    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    created_mutex = new (std::nothrow) pt_recursive_mutex();
    if (created_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = created_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete created_mutex;
        return (initialize_error);
    }
    *mutex_pointer = created_mutex;
    return (FT_ERR_SUCCESS);
}

static void destroy_recursive_mutex(pt_recursive_mutex **mutex_pointer)
{
    int destroy_error;

    if (mutex_pointer == ft_nullptr || *mutex_pointer == ft_nullptr)
        return ;
    destroy_error = (*mutex_pointer)->destroy();
    if (destroy_error == FT_ERR_SUCCESS)
    {
        delete *mutex_pointer;
        *mutex_pointer = ft_nullptr;
    }
    return ;
}

static int lock_recursive_mutex_if_valid(pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->lock());
}

static int unlock_recursive_mutex_if_valid(pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->unlock());
}

ft_render_screen_size ft_render_get_primary_screen_size(void)
{
    ft_render_platform_result  platform_result;
    ft_render_screen_size      size;

    size.width = 0;
    size.height = 0;

    platform_result = ft_render_platform_get_primary_screen_size(&size);
    if (platform_result.error_code != ft_render_ok)
        return (size);
    
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
    this->_initialized_state = ft_render_window::_state_uninitialized;
    return ;
}

ft_render_window::~ft_render_window(void)
{
    if (this->_initialized_state == ft_render_window::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_render_window::~ft_render_window",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_render_window::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_render_window::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_render_window lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_render_window::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_render_window::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_render_window::initialize(void)
{
    if (this->_initialized_state == ft_render_window::_state_initialized)
    {
        this->abort_lifecycle_error("ft_render_window::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_framebuffer.width = 0;
    this->_framebuffer.height = 0;
    this->_framebuffer.pixels = NULL;
    this->_is_initialized = false;
    this->_should_close = false;
    this->_platform_state = NULL;
    this->_initialized_state = ft_render_window::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_render_window::initialize(const ft_render_window &other)
{
    if (other._initialized_state == ft_render_window::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_render_window::initialize(const ft_render_window &) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_render_window::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (other._is_initialized == true)
        return (FT_ERR_INVALID_OPERATION);
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_should_close = other._should_close;
    return (FT_ERR_SUCCESS);
}

int ft_render_window::initialize(ft_render_window &&other)
{
    int initialization_error;
    int move_error;

    if (other._initialized_state == ft_render_window::_state_uninitialized)
    {
        other.abort_lifecycle_error(
            "ft_render_window::initialize(ft_render_window &&) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_render_window::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_render_window::destroy(void)
{
    if (this->_initialized_state != ft_render_window::_state_initialized)
    {
        this->abort_lifecycle_error("ft_render_window::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_is_initialized == true)
        this->shutdown();
    this->disable_thread_safety();
    this->_initialized_state = ft_render_window::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_render_window::move(ft_render_window &other)
{
    this->abort_if_not_initialized("ft_render_window::move destination");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialized_state == ft_render_window::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_render_window::move source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_render_window::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_framebuffer = other._framebuffer;
    this->_is_initialized = other._is_initialized;
    this->_should_close = other._should_close;
    this->_platform_state = other._platform_state;
    other._framebuffer.width = 0;
    other._framebuffer.height = 0;
    other._framebuffer.pixels = ft_nullptr;
    other._is_initialized = false;
    other._should_close = false;
    other._platform_state = ft_nullptr;
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_render_window::runtime_mutex(void)
{
    this->abort_if_not_initialized("ft_render_window::runtime_mutex");
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

    this->abort_if_not_initialized("ft_render_window::initialize(desc)");
    lock_status = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_status != FT_ERR_SUCCESS)
        return (lock_status);
    if (this->_is_initialized == true)
    {
        this->abort_lifecycle_error("ft_render_window::initialize(desc)",
            "called while render window is already initialized");
        unlock_status = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESS)
            return (unlock_status);
        return (FT_ERR_INVALID_STATE);
    }
    if (desc.width <= 0 || desc.height <= 0 || desc.title == NULL)
    {
        unlock_status = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESS)
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
        unlock_status = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_status != FT_ERR_SUCCESS)
            return (unlock_status);
        return (platform_result.error_code);
    }
    this->_is_initialized = true;
    this->_should_close = false;
    unlock_status = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_status != FT_ERR_SUCCESS)
        return (unlock_status);
    return (ft_render_ok);
}

void ft_render_window::shutdown(void)
{
    int                        lock_error;
    int                        unlock_error;

    this->abort_if_not_initialized("ft_render_window::shutdown");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_is_initialized != false)
    {
        (void)ft_render_platform_destroy_window(
            &this->_platform_state,
            &this->_framebuffer
        );

        this->_platform_state = NULL;
        this->_framebuffer.width = 0;
        this->_framebuffer.height = 0;
        this->_framebuffer.pixels = NULL;

        this->_is_initialized = false;
        this->_should_close = true;
    }
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    return ;
}

int ft_render_window::poll_events(void)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    this->abort_if_not_initialized("ft_render_window::poll_events");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_poll_events(
        this->_platform_state,
        &this->_should_close
    );
    if (platform_result.error_code != ft_render_ok)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_render_ok);
}

int ft_render_window::present(void)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    this->abort_if_not_initialized("ft_render_window::present");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_present(
        this->_platform_state,
        &this->_framebuffer
    );
    if (platform_result.error_code != ft_render_ok)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_render_ok);
}

ft_render_framebuffer &ft_render_window::framebuffer(void)
{
    this->abort_if_not_initialized("ft_render_window::framebuffer");
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

    this->abort_if_not_initialized("ft_render_window::clear");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    if (this->_is_initialized == false)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
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
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_render_ok);
}

int ft_render_window::put_pixel(int x, int y, uint32_t color)
{
    int lock_error;
    int unlock_error;
    int width;
    int height;
    int index;

    this->abort_if_not_initialized("ft_render_window::put_pixel");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    width = this->_framebuffer.width;
    height = this->_framebuffer.height;
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_render_error_invalid_argument);
    }
    index = (y * width) + x;
    this->_framebuffer.pixels[index] = color;
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_render_ok);
}

int ft_render_window::set_fullscreen(bool enabled)
{
    ft_render_platform_result  platform_result;
    int                        lock_error;
    int                        unlock_error;

    this->abort_if_not_initialized("ft_render_window::set_fullscreen");
    lock_error = lock_recursive_mutex_if_valid(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_is_initialized == false)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (ft_render_error_not_initialized);
    }
    platform_result = ft_render_platform_set_fullscreen(
        this->_platform_state,
        enabled
    );
    if (platform_result.error_code != ft_render_ok)
    {
        unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (platform_result.error_code);
    }
    unlock_error = unlock_recursive_mutex_if_valid(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (ft_render_ok);
}

bool ft_render_window::should_close(void) const
{
    this->abort_if_not_initialized("ft_render_window::should_close");
    return (this->_should_close);
}

int ft_render_window::prepare_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_render_window::prepare_thread_safety");
    if (this->_mutex)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = create_recursive_mutex(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void ft_render_window::teardown_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_render_window::teardown_thread_safety");
    destroy_recursive_mutex(&this->_mutex);
    return ;
}

int ft_render_window::enable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_render_window::enable_thread_safety");
    return (this->prepare_thread_safety());
}

void ft_render_window::disable_thread_safety() noexcept
{
    this->abort_if_not_initialized("ft_render_window::disable_thread_safety");
    this->teardown_thread_safety();
    return ;
}

bool ft_render_window::is_thread_safe_enabled() const noexcept
{
    this->abort_if_not_initialized("ft_render_window::is_thread_safe_enabled");
    return (this->_mutex != ft_nullptr);
}
