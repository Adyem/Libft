#include "../PThread/pthread_internal.hpp"
#include "game_map3d.hpp"
#include "game_pathfinding.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_map3d::_last_error = FT_ERR_SUCCESS;

ft_map3d::ft_map3d()
    : _data(ft_nullptr), _width(0), _height(0), _depth(0),
      _initial_value(0), _mutex(ft_nullptr),
      _initialized_state(ft_map3d::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map3d::~ft_map3d()
{
    if (this->_initialized_state == ft_map3d::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_map3d::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_map3d::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_map3d lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_map3d::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_map3d::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

void ft_map3d::set_error(int error_code) const noexcept
{
    ft_map3d::_last_error = error_code;
    return ;
}

int ft_map3d::get_error() const noexcept
{
    return (ft_map3d::_last_error);
}

const char *ft_map3d::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

int ft_map3d::initialize()
{
    if (this->_initialized_state == ft_map3d::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map3d::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    return (this->initialize(this->_width, this->_height, this->_depth,
        this->_initial_value));
}

int ft_map3d::initialize(size_t width, size_t height, size_t depth, int value)
{
    if (this->_initialized_state == ft_map3d::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map3d::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_width = width;
    this->_height = height;
    this->_depth = depth;
    this->_initial_value = value;
    this->_data = ft_nullptr;
    this->allocate(width, height, depth, value);
    this->_initialized_state = ft_map3d::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_map3d::destroy()
{
    int disable_error;

    if (this->_initialized_state != ft_map3d::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->deallocate();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_map3d::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_map3d::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_map3d::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_map3d::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_map3d::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_map3d::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_map3d::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int unlock_error;

    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    return (unlock_error);
}

int ft_map3d::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_map3d::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_map3d::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_map3d::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

void ft_map3d::resize(size_t width, size_t height, size_t depth, int value)
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_map3d::resize");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->deallocate();
    this->_width = width;
    this->_height = height;
    this->_depth = depth;
    this->allocate(width, height, depth, value);
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_map3d::get(size_t x, size_t y, size_t z) const
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_map3d::get");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    if (this->_data == ft_nullptr || x >= this->_width || y >= this->_height
        || z >= this->_depth)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    int value;

    value = this->_data[z][y][x];
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_map3d::set(size_t x, size_t y, size_t z, int value)
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_map3d::set");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_data != ft_nullptr && x < this->_width && y < this->_height
        && z < this->_depth)
        this->_data[z][y][x] = value;
    else
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_map3d::is_obstacle(size_t x, size_t y, size_t z) const
{
    int value;

    value = this->get(x, y, z);
    if (value != 0)
        return (1);
    return (0);
}

void ft_map3d::toggle_obstacle(size_t x, size_t y, size_t z,
    ft_pathfinding *listener)
{
    bool lock_acquired;
    int lock_error;
    int new_value;

    this->abort_if_not_initialized("ft_map3d::toggle_obstacle");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_data == ft_nullptr || x >= this->_width || y >= this->_height
        || z >= this->_depth)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return ;
    }
    if (this->_data[z][y][x] == 0)
        this->_data[z][y][x] = 1;
    else
        this->_data[z][y][x] = 0;
    new_value = this->_data[z][y][x];
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    if (listener != ft_nullptr)
        listener->update_obstacle(x, y, z, new_value);
    return ;
}

size_t ft_map3d::get_width() const
{
    bool lock_acquired;
    int lock_error;
    size_t width_value;

    this->abort_if_not_initialized("ft_map3d::get_width");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    width_value = this->_width;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (width_value);
}

size_t ft_map3d::get_height() const
{
    bool lock_acquired;
    int lock_error;
    size_t height_value;

    this->abort_if_not_initialized("ft_map3d::get_height");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    height_value = this->_height;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (height_value);
}

size_t ft_map3d::get_depth() const
{
    bool lock_acquired;
    int lock_error;
    size_t depth_value;

    this->abort_if_not_initialized("ft_map3d::get_depth");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    depth_value = this->_depth;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (depth_value);
}

void ft_map3d::allocate(size_t width, size_t height, size_t depth, int value)
{
    size_t index_depth;

    this->_width = width;
    this->_height = height;
    this->_depth = depth;
    this->_data = ft_nullptr;
    if (width == 0 || height == 0 || depth == 0)
        return ;
    this->_data = static_cast<int ***>(cma_malloc(sizeof(int **) * depth));
    if (this->_data == ft_nullptr)
        return ;
    index_depth = 0;
    while (index_depth < depth)
    {
        this->_data[index_depth] = ft_nullptr;
        index_depth += 1;
    }
    index_depth = 0;
    while (index_depth < depth)
    {
        size_t index_height;

        this->_data[index_depth] = static_cast<int **>(cma_malloc(sizeof(int *) * height));
        if (this->_data[index_depth] == ft_nullptr)
        {
            this->deallocate();
            return ;
        }
        index_height = 0;
        while (index_height < height)
        {
            size_t index_width;

            this->_data[index_depth][index_height] = static_cast<int *>(
                    cma_malloc(sizeof(int) * width));
            if (this->_data[index_depth][index_height] == ft_nullptr)
            {
                this->deallocate();
                return ;
            }
            index_width = 0;
            while (index_width < width)
            {
                this->_data[index_depth][index_height][index_width] = value;
                index_width += 1;
            }
            index_height += 1;
        }
        index_depth += 1;
    }
    return ;
}

void ft_map3d::deallocate()
{
    size_t index_depth;

    if (this->_data == ft_nullptr)
    {
        this->_width = 0;
        this->_height = 0;
        this->_depth = 0;
        return ;
    }
    index_depth = 0;
    while (index_depth < this->_depth)
    {
        size_t index_height;

        if (this->_data[index_depth] != ft_nullptr)
        {
            index_height = 0;
            while (index_height < this->_height)
            {
                if (this->_data[index_depth][index_height] != ft_nullptr)
                    cma_free(this->_data[index_depth][index_height]);
                index_height += 1;
            }
            cma_free(this->_data[index_depth]);
        }
        index_depth += 1;
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_width = 0;
    this->_height = 0;
    this->_depth = 0;
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_map3d::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
