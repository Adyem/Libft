#ifndef GAME_PATHFINDING_HPP
# define GAME_PATHFINDING_HPP

#include "game_map3d.hpp"
#include "../Template/vector.hpp"
#include "../Template/graph.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_path_step
{
    private:
        size_t              _x;
        size_t              _y;
        size_t              _z;
        pt_recursive_mutex *_mutex;
        uint8_t             _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;
        int     lock_internal(bool *lock_acquired) const noexcept;
        int     unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_path_step() noexcept;
        ~ft_path_step() noexcept;
        ft_path_step(const ft_path_step &other) noexcept;
        ft_path_step &operator=(const ft_path_step &other) noexcept;
        ft_path_step(ft_path_step &&other) noexcept;
        ft_path_step &operator=(ft_path_step &&other) noexcept;

        int     initialize() noexcept;
        int     destroy() noexcept;
        int     enable_thread_safety() noexcept;
        int     disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;

        int     set_coordinates(size_t x, size_t y, size_t z) noexcept;
        int     set_x(size_t x) noexcept;
        int     set_y(size_t y) noexcept;
        int     set_z(size_t z) noexcept;
        size_t  get_x() const noexcept;
        size_t  get_y() const noexcept;
        size_t  get_z() const noexcept;
        int     get_error() const noexcept;
        const char *get_error_str() const noexcept;
        void    reset_system_error() const noexcept;
};

class ft_pathfinding
{
    private:
        ft_vector<ft_path_step> _current_path;
        bool                    _needs_replan;
        pt_mutex               *_mutex;
        uint8_t                 _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;
        int     lock_internal(bool *lock_acquired) const noexcept;
        int     unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_pathfinding() noexcept;
        ~ft_pathfinding();
        ft_pathfinding(const ft_pathfinding &other) = delete;
        ft_pathfinding &operator=(const ft_pathfinding &other) = delete;
        ft_pathfinding(ft_pathfinding &&other) = delete;
        ft_pathfinding &operator=(ft_pathfinding &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int astar_grid(const ft_map3d &grid,
            size_t start_x, size_t start_y, size_t start_z,
            size_t goal_x, size_t goal_y, size_t goal_z,
            ft_vector<ft_path_step> &out_path) const noexcept;

        int dijkstra_graph(const ft_graph<int> &graph,
            size_t start_vertex, size_t goal_vertex,
            ft_vector<size_t> &out_path) const noexcept;

        void    update_obstacle(size_t x, size_t y, size_t z, int value) noexcept;
        int     recalculate_path(const ft_map3d &grid,
            size_t start_x, size_t start_y, size_t start_z,
            size_t goal_x, size_t goal_y, size_t goal_z,
            ft_vector<ft_path_step> &out_path) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
