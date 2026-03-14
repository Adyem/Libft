#ifndef GAME_MAP3D_HPP
# define GAME_MAP3D_HPP

#include <cstddef>
#include <stdint.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_pathfinding;

class ft_map3d
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t       ***_data;
        ft_size_t      _width;
        ft_size_t      _height;
        ft_size_t      _depth;
        int32_t         _initial_value;
        pt_recursive_mutex   *_mutex;
        uint8_t     _initialised_state;
        static thread_local int32_t _last_error;
        void    set_error(int32_t error_code) const noexcept;
        int32_t     lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t     unlock_internal(ft_bool lock_acquired) const noexcept;

        void    allocate(ft_size_t width, ft_size_t height, ft_size_t depth, int32_t value);
        void    deallocate();

    public:
        ft_map3d();
        ft_map3d(const ft_map3d &other);
        ft_map3d(ft_map3d &&other);
        ~ft_map3d();
        ft_map3d &operator=(const ft_map3d &other) = delete;
        ft_map3d &operator=(ft_map3d &&other) = delete;

        int32_t     initialize();
        int32_t     initialize(ft_size_t width, ft_size_t height, ft_size_t depth, int32_t value);
        int32_t     move(ft_map3d &other);
        int32_t     destroy();
        int32_t     enable_thread_safety() noexcept;
        int32_t     disable_thread_safety() noexcept;
        ft_bool    is_thread_safe() const noexcept;
        int32_t     lock(ft_bool *lock_acquired) const noexcept;
        void    unlock(ft_bool lock_acquired) const noexcept;

        int32_t     get_error() const noexcept;
        const char *get_error_str() const noexcept;

        void    resize(ft_size_t width, ft_size_t height, ft_size_t depth, int32_t value = 0);
        int32_t     get(ft_size_t x, ft_size_t y, ft_size_t z) const;
        void    set(ft_size_t x, ft_size_t y, ft_size_t z, int32_t value);
        ft_bool    is_obstacle(ft_size_t x, ft_size_t y, ft_size_t z) const;
        void    toggle_obstacle(ft_size_t x, ft_size_t y, ft_size_t z,
                    ft_pathfinding *listener = ft_nullptr);
        ft_size_t  get_width() const;
        ft_size_t  get_height() const;
        ft_size_t  get_depth() const;

};

#endif
