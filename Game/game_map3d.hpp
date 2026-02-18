#ifndef GAME_MAP3D_HPP
# define GAME_MAP3D_HPP

#include <cstddef>
#include <stdint.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

class ft_pathfinding;

class ft_map3d
{
    private:
        int       ***_data;
        size_t      _width;
        size_t      _height;
        size_t      _depth;
        int         _initial_value;
        pt_mutex   *_mutex;
        uint8_t     _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;
        int     lock_internal(bool *lock_acquired) const noexcept;
        int     unlock_internal(bool lock_acquired) const noexcept;

        void    allocate(size_t width, size_t height, size_t depth, int value);
        void    deallocate();

    public:
        ft_map3d(size_t width = 0, size_t height = 0, size_t depth = 0,
            int value = 0);
        ~ft_map3d();
        ft_map3d(const ft_map3d &other) = delete;
        ft_map3d &operator=(const ft_map3d &other) = delete;
        ft_map3d(ft_map3d &&other) = delete;
        ft_map3d &operator=(ft_map3d &&other) = delete;

        int     initialize();
        int     destroy();
        int     enable_thread_safety() noexcept;
        int     disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;
        int     lock(bool *lock_acquired) const noexcept;
        void    unlock(bool lock_acquired) const noexcept;

        void    resize(size_t width, size_t height, size_t depth, int value = 0);
        int     get(size_t x, size_t y, size_t z) const;
        void    set(size_t x, size_t y, size_t z, int value);
        int     is_obstacle(size_t x, size_t y, size_t z) const;
        void    toggle_obstacle(size_t x, size_t y, size_t z,
                    ft_pathfinding *listener = ft_nullptr);
        size_t  get_width() const;
        size_t  get_height() const;
        size_t  get_depth() const;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
