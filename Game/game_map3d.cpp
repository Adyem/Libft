#include "map3d.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_map3d::ft_map3d(size_t width, size_t height, size_t depth, int value)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0), _error(ER_SUCCESS)
{
    this->allocate(width, height, depth, value);
    return ;
}

ft_map3d::~ft_map3d()
{
    this->deallocate();
    return ;
}

void ft_map3d::resize(size_t width, size_t height, size_t depth, int value)
{
    this->deallocate();
    this->allocate(width, height, depth, value);
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
    return ;
}

size_t ft_map3d::get_width() const
{
    return (this->_width);
}

size_t ft_map3d::get_height() const
{
    return (this->_height);
}

size_t ft_map3d::get_depth() const
{
    return (this->_depth);
}

void ft_map3d::allocate(size_t width, size_t height, size_t depth, int value)
{
    this->_error = ER_SUCCESS;
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
    return ;
}

void ft_map3d::deallocate()
{
    if (!this->_data)
        return ;
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
    this->_error = ER_SUCCESS;
    return ;
}
