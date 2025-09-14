#ifndef GAME_MAP3D_HPP
# define GAME_MAP3D_HPP

#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"

class ft_pathfinding;

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
        ft_map3d(const ft_map3d &other);
        ft_map3d &operator=(const ft_map3d &other);
        ft_map3d(ft_map3d &&other) noexcept;
        ft_map3d &operator=(ft_map3d &&other) noexcept;

        void    resize(size_t width, size_t height, size_t depth, int value = 0);
        int     get(size_t x, size_t y, size_t z) const;
        void    set(size_t x, size_t y, size_t z, int value);
        int     is_obstacle(size_t x, size_t y, size_t z) const;
        void    toggle_obstacle(size_t x, size_t y, size_t z, ft_pathfinding *listener = ft_nullptr);
        size_t  get_width() const;
        size_t  get_height() const;
        size_t  get_depth() const;
        int     get_error() const;
        const char *get_error_str() const;
};

#endif
