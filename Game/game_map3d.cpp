#include "game_map3d.hpp"
#include "game_pathfinding.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

void ft_map3d::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void ft_map3d::restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_map3d::lock_pair(const ft_map3d &first, const ft_map3d &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_map3d *ordered_first;
    const ft_map3d *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_map3d *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
        {
            if (swapped == false)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_map3d::sleep_backoff();
    }
}

ft_map3d::ft_map3d(size_t width, size_t height, size_t depth, int value)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS),
      _mutex()
{
    this->allocate(width, height, depth, value);
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

ft_map3d::~ft_map3d()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    this->deallocate();
    ft_map3d::restore_errno(guard, entry_errno);
    return ;
}

ft_map3d::ft_map3d(const ft_map3d &other)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS),
      _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_map3d::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->allocate(other._width, other._height, other._depth, 0);
    if (this->_error == ER_SUCCESS && this->_data)
    {
        size_t z = 0;
        while (z < this->_depth)
        {
            size_t y = 0;
            while (y < this->_height)
            {
                size_t x = 0;
                while (x < this->_width)
                {
                    this->_data[z][y][x] = other._data[z][y][x];
                    ++x;
                }
                ++y;
            }
            ++z;
        }
    }
    this->set_error(this->_error == ER_SUCCESS ? other._error : this->_error);
    ft_map3d::restore_errno(other_guard, entry_errno);
    return ;
}

ft_map3d &ft_map3d::operator=(const ft_map3d &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_map3d::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_map3d::restore_errno(this_guard, entry_errno);
        ft_map3d::restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->deallocate();
    this->allocate(other._width, other._height, other._depth, 0);
    if (this->_error == ER_SUCCESS && this->_data)
    {
        size_t z = 0;
        while (z < this->_depth)
        {
            size_t y = 0;
            while (y < this->_height)
            {
                size_t x = 0;
                while (x < this->_width)
                {
                    this->_data[z][y][x] = other._data[z][y][x];
                    ++x;
                }
                ++y;
            }
            ++z;
        }
    }
    this->set_error(this->_error == ER_SUCCESS ? other._error : this->_error);
    ft_map3d::restore_errno(this_guard, entry_errno);
    ft_map3d::restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_map3d::ft_map3d(ft_map3d &&other) noexcept
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS),
      _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_map3d::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_data = other._data;
    this->_width = other._width;
    this->_height = other._height;
    this->_depth = other._depth;
    this->_error = other._error;
    other._data = ft_nullptr;
    other._width = 0;
    other._height = 0;
    other._depth = 0;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    ft_map3d::restore_errno(other_guard, entry_errno);
    return ;
}

ft_map3d &ft_map3d::operator=(ft_map3d &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_map3d::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_map3d::restore_errno(this_guard, entry_errno);
        ft_map3d::restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->deallocate();
    this->_data = other._data;
    this->_width = other._width;
    this->_height = other._height;
    this->_depth = other._depth;
    this->_error = other._error;
    other._data = ft_nullptr;
    other._width = 0;
    other._height = 0;
    other._depth = 0;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    ft_map3d::restore_errno(this_guard, entry_errno);
    ft_map3d::restore_errno(other_guard, entry_errno);
    return (*this);
}

void ft_map3d::resize(size_t width, size_t height, size_t depth, int value)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    this->deallocate();
    this->allocate(width, height, depth, value);
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return ;
}

int ft_map3d::get_error() const
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_map3d *>(this)->set_error(error_code);
    ft_map3d::restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_map3d::get_error_str() const
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_map3d *>(this)->set_error(error_code);
    ft_map3d::restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_map3d::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

size_t ft_map3d::index(size_t x, size_t y, size_t z) const
{
    return (x + y * this->_width + z * this->_width * this->_height);
}

int ft_map3d::get(size_t x, size_t y, size_t z) const
{
    int entry_errno;
    int value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        const_cast<ft_map3d *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_data[z][y][x];
    const_cast<ft_map3d *>(this)->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return (value);
}

void ft_map3d::set(size_t x, size_t y, size_t z, int value)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    this->_data[z][y][x] = value;
    this->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return ;
}

int ft_map3d::is_obstacle(size_t x, size_t y, size_t z) const
{
    int entry_errno;
    int value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        const_cast<ft_map3d *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_data[z][y][x];
    const_cast<ft_map3d *>(this)->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    if (value != 0)
        return (1);
    return (0);
}

void ft_map3d::toggle_obstacle(size_t x, size_t y, size_t z, ft_pathfinding *listener)
{
    int entry_errno;
    int new_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        ft_map3d::restore_errno(guard, entry_errno);
        return ;
    }
    if (this->_data[z][y][x] == 0)
        this->_data[z][y][x] = 1;
    else
        this->_data[z][y][x] = 0;
    new_value = this->_data[z][y][x];
    this->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    if (listener)
        listener->update_obstacle(x, y, z, new_value);
    return ;
}

size_t ft_map3d::get_width() const
{
    int entry_errno;
    size_t width_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    width_value = this->_width;
    const_cast<ft_map3d *>(this)->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return (width_value);
}

size_t ft_map3d::get_height() const
{
    int entry_errno;
    size_t height_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    height_value = this->_height;
    const_cast<ft_map3d *>(this)->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return (height_value);
}

size_t ft_map3d::get_depth() const
{
    int entry_errno;
    size_t depth_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_map3d *>(this)->set_error(guard.get_error());
        ft_map3d::restore_errno(guard, entry_errno);
        return (0);
    }
    depth_value = this->_depth;
    const_cast<ft_map3d *>(this)->set_error(ER_SUCCESS);
    ft_map3d::restore_errno(guard, entry_errno);
    return (depth_value);
}

void ft_map3d::allocate(size_t width, size_t height, size_t depth, int value)
{
    this->set_error(ER_SUCCESS);
    this->_width = width;
    this->_height = height;
    this->_depth = depth;
    if (width == 0 || height == 0 || depth == 0)
    {
        this->_data = ft_nullptr;
        return ;
    }
    this->_data = static_cast<int***>(cma_malloc(sizeof(int**) * depth));
    if (!this->_data)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    size_t z = 0;
    while (z < depth)
    {
        this->_data[z] = ft_nullptr;
        ++z;
    }
    z = 0;
    while (z < depth)
    {
        this->_data[z] = static_cast<int**>(cma_malloc(sizeof(int*) * height));
        if (!this->_data[z])
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->deallocate();
            return ;
        }
        size_t y = 0;
        while (y < height)
        {
            this->_data[z][y] = ft_nullptr;
            ++y;
        }
        y = 0;
        while (y < height)
        {
            this->_data[z][y] = static_cast<int*>(cma_malloc(sizeof(int) * width));
            if (!this->_data[z][y])
            {
                this->set_error(FT_ERR_NO_MEMORY);
                this->deallocate();
                return ;
            }
            size_t x = 0;
            while (x < width)
            {
                this->_data[z][y][x] = value;
                ++x;
            }
            ++y;
        }
        ++z;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_map3d::deallocate()
{
    if (!this->_data)
    {
        this->_width = 0;
        this->_height = 0;
        this->_depth = 0;
        if (this->_error == ER_SUCCESS)
            this->set_error(ER_SUCCESS);
        return ;
    }
    size_t z = 0;
    while (z < this->_depth)
    {
        size_t y = 0;
        while (y < this->_height)
        {
            cma_free(this->_data[z][y]);
            ++y;
        }
        cma_free(this->_data[z]);
        ++z;
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_width = 0;
    this->_height = 0;
    this->_depth = 0;
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}
