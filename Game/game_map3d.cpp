#include "game_map3d.hpp"
#include "game_pathfinding.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_map3d::ft_map3d(size_t width, size_t height, size_t depth, int value)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS)
{
    this->allocate(width, height, depth, value);
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

ft_map3d::~ft_map3d()
{
    this->deallocate();
    return ;
}

ft_map3d::ft_map3d(const ft_map3d &other)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS)
{
    this->allocate(other._width, other._height, other._depth, 0);
    if (this->_error != ER_SUCCESS)
        return ;
    if (this->_data)
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
    this->set_error(other._error);
    return ;
}

ft_map3d &ft_map3d::operator=(const ft_map3d &other)
{
    if (this != &other)
    {
        this->deallocate();
        this->allocate(other._width, other._height, other._depth, 0);
        if (this->_error != ER_SUCCESS)
            return (*this);
        if (this->_data)
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
        this->set_error(other._error);
    }
    return (*this);
}

ft_map3d::ft_map3d(ft_map3d &&other) noexcept
    : _data(other._data), _width(other._width), _height(other._height), _depth(other._depth), _error(other._error)
{
    other._data = ft_nullptr;
    other._width = 0;
    other._height = 0;
    other._depth = 0;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    return ;
}

ft_map3d &ft_map3d::operator=(ft_map3d &&other) noexcept
{
    if (this != &other)
    {
        this->deallocate();
        this->_data = other._data;
        this->_width = other._width;
        this->_height = other._height;
        this->_depth = other._depth;
        this->set_error(other._error);
        other._data = ft_nullptr;
        other._width = 0;
        other._height = 0;
        other._depth = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

void ft_map3d::resize(size_t width, size_t height, size_t depth, int value)
{
    this->deallocate();
    this->allocate(width, height, depth, value);
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

int ft_map3d::get_error() const
{
    return (this->_error);
}

const char *ft_map3d::get_error_str() const
{
    return (ft_strerror(this->_error));
}

void ft_map3d::set_error(int err) const
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
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        const_cast<ft_map3d*>(this)->set_error(MAP3D_OUT_OF_BOUNDS);
        return (0);
    }
    const_cast<ft_map3d*>(this)->set_error(ER_SUCCESS);
    return (this->_data[z][y][x]);
}

void ft_map3d::set(size_t x, size_t y, size_t z, int value)
{
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        this->set_error(MAP3D_OUT_OF_BOUNDS);
        return ;
    }
    this->_data[z][y][x] = value;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_map3d::is_obstacle(size_t x, size_t y, size_t z) const
{
    int value = this->get(x, y, z);
    if (this->_error != ER_SUCCESS)
        return (0);
    if (value != 0)
        return (1);
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_map3d::toggle_obstacle(size_t x, size_t y, size_t z, ft_pathfinding *listener)
{
    if (!this->_data || x >= this->_width || y >= this->_height || z >= this->_depth)
    {
        this->set_error(MAP3D_OUT_OF_BOUNDS);
        return ;
    }
    if (this->_data[z][y][x] == 0)
        this->_data[z][y][x] = 1;
    else
        this->_data[z][y][x] = 0;
    if (listener)
        listener->update_obstacle(x, y, z, this->_data[z][y][x]);
    this->set_error(ER_SUCCESS);
    return ;
}

size_t ft_map3d::get_width() const
{
    const_cast<ft_map3d*>(this)->set_error(ER_SUCCESS);
    return (this->_width);
}

size_t ft_map3d::get_height() const
{
    const_cast<ft_map3d*>(this)->set_error(ER_SUCCESS);
    return (this->_height);
}

size_t ft_map3d::get_depth() const
{
    const_cast<ft_map3d*>(this)->set_error(ER_SUCCESS);
    return (this->_depth);
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
        this->set_error(MAP3D_ALLOC_FAIL);
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
            this->set_error(MAP3D_ALLOC_FAIL);
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
                this->set_error(MAP3D_ALLOC_FAIL);
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
