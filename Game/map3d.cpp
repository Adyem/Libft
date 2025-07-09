#include "map3d.hpp"

ft_map3d::ft_map3d(size_t width, size_t height, size_t depth, int value)
    : _data(ft_nullptr), _width(0), _height(0), _depth(0)
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

size_t ft_map3d::index(size_t x, size_t y, size_t z) const
{
    return (x + y * this->_width + z * this->_width * this->_height);
}

int ft_map3d::get(size_t x, size_t y, size_t z) const
{
    return (this->_data[z][y][x]);
}

void ft_map3d::set(size_t x, size_t y, size_t z, int value)
{
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
        return ;
    for (size_t z = 0; z < depth; z++)
    {
        this->_data[z] = static_cast<int**>(cma_malloc(sizeof(int*) * height));
        if (!this->_data[z])
            return ;
        for (size_t y = 0; y < height; y++)
        {
            this->_data[z][y] = static_cast<int*>(cma_malloc(sizeof(int) * width));
            if (!this->_data[z][y])
                return ;
            for (size_t x = 0; x < width; x++)
                this->_data[z][y][x] = value;
        }
    }
    return ;
}

void ft_map3d::deallocate()
{
    if (!this->_data)
        return ;
    for (size_t z = 0; z < this->_depth; z++)
    {
        for (size_t y = 0; y < this->_height; y++)
            cma_free(this->_data[z][y]);
        cma_free(this->_data[z]);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_width = 0;
    this->_height = 0;
    this->_depth = 0;
    return ;
}
