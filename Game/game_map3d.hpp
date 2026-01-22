#ifndef GAME_MAP3D_HPP
# define GAME_MAP3D_HPP

#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_pathfinding;

class ft_map3d
{
    private:
        int     ***_data;
        size_t  _width;
        size_t  _height;
        size_t  _depth;
        mutable int    _error;
        mutable pt_mutex    _mutex;
        mutable ft_operation_error_stack _operation_errors;

        void    set_error(int err) const noexcept;
        void    record_operation_error(int error_code) const noexcept;

        static void finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept;
        static void sleep_backoff() noexcept;
        static int lock_pair(const ft_map3d &first, const ft_map3d &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard) noexcept;

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
        ft_operation_error_stack *get_operation_error_stack_for_validation() noexcept;
        int     last_operation_error() const noexcept;
        const char *last_operation_error_str() const noexcept;
        int     operation_error_at(ft_size_t index) const noexcept;
        const char *operation_error_str_at(ft_size_t index) const noexcept;
        void    pop_operation_errors() noexcept;
        int     pop_oldest_operation_error() noexcept;
        int     pop_newest_operation_error() noexcept;
};

#endif
