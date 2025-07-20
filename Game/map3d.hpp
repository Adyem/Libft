#ifndef MAP3D_HPP
# define MAP3D_HPP

#include <cstddef>

class ft_map3d
{
    private:
        int     ***_data;
        size_t  _width;
        size_t  _height;
        size_t  _depth;
        mutable int    _error;

        void    set_error(int err) const;


        void    allocate(size_t width, size_t height, size_t depth, int value);
        void    deallocate();
        size_t  index(size_t x, size_t y, size_t z) const;

    public:
        ft_map3d(size_t width = 0, size_t height = 0, size_t depth = 0, int value = 0);
        ~ft_map3d();

        ft_map3d(const ft_map3d&) = delete;
        ft_map3d &operator=(const ft_map3d&) = delete;

        void    resize(size_t width, size_t height, size_t depth, int value = 0);
        int     get(size_t x, size_t y, size_t z) const;
        void    set(size_t x, size_t y, size_t z, int value);
        size_t  get_width() const;
        size_t  get_height() const;
        size_t  get_depth() const;
        int     get_error() const;
        const char *get_error_str() const;
};

#endif
