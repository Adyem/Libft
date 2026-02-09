#ifndef GAME_PATHFINDING_HPP
# define GAME_PATHFINDING_HPP

#include "game_map3d.hpp"
#include "../Template/vector.hpp"
#include "../Template/graph.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#ifdef LIBFT_TEST_BUILD
#include "../PThread/pthread.hpp"
#endif
#include "../PThread/unique_lock.hpp"

class ft_path_step
{
    private:
        size_t              _x;
        size_t              _y;
        size_t              _z;
        mutable int         _error_code;
        mutable int         _system_error_code;
        mutable pt_recursive_mutex    _mutex;

        void    set_error(int error) const noexcept;
        void    set_system_error(int error) const noexcept;
        static int lock_pair(const ft_path_step &first, const ft_path_step &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept;

#ifdef LIBFT_TEST_BUILD
        friend struct ft_path_step_test_helper;
#endif

    public:
        ft_path_step() noexcept;
        ~ft_path_step() noexcept;
        ft_path_step(const ft_path_step &other) noexcept;
        ft_path_step &operator=(const ft_path_step &other) noexcept;
        ft_path_step(ft_path_step &&other) noexcept;
        ft_path_step &operator=(ft_path_step &&other) noexcept;

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

#ifdef LIBFT_TEST_BUILD
struct ft_path_step_test_helper
{
    static int lock(ft_path_step &step) noexcept;
    static int unlock(ft_path_step &step) noexcept;
    static bool is_locked(const ft_path_step &step) noexcept;
    static bool is_owned_by_thread(const ft_path_step &step, pthread_t thread_id) noexcept;
    static int get_mutex_error(const ft_path_step &step) noexcept;
};

inline int ft_path_step_test_helper::lock(ft_path_step &step) noexcept
{
    int result;

    result = step._mutex.lock();
    if (step._mutex.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = step._mutex.get_error();
        return (step._mutex.get_error());
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

inline int ft_path_step_test_helper::unlock(ft_path_step &step) noexcept
{
    int result;

    result = step._mutex.unlock();
    if (step._mutex.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = step._mutex.get_error();
        return (step._mutex.get_error());
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

inline bool ft_path_step_test_helper::is_locked(const ft_path_step &step) noexcept
{
    bool locked;

    locked = step._mutex.lockState();
    ft_errno = FT_ERR_SUCCESSS;
    return (locked);
}

inline bool ft_path_step_test_helper::is_owned_by_thread(const ft_path_step &step, pthread_t thread_id) noexcept
{
    bool owned;

    owned = step._mutex.is_owned_by_thread(thread_id);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (false);
    ft_errno = FT_ERR_SUCCESSS;
    return (owned);
}

inline int ft_path_step_test_helper::get_mutex_error(const ft_path_step &step) noexcept
{
    int error_code;

    error_code = step._mutex.get_error();
    ft_errno = error_code;
    return (error_code);
}
#endif

class ft_pathfinding
{
    private:
        mutable int         _error_code;
        ft_vector<ft_path_step> _current_path;
        bool                _needs_replan;
        mutable pt_mutex    _mutex;

        void    set_error(int error) const noexcept;
        static void finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept;
        static void sleep_backoff() noexcept;
        static int lock_pair(const ft_pathfinding &first, const ft_pathfinding &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
        ft_pathfinding() noexcept;
        ~ft_pathfinding();
        ft_pathfinding(const ft_pathfinding &other) noexcept;
        ft_pathfinding &operator=(const ft_pathfinding &other) noexcept;
        ft_pathfinding(ft_pathfinding &&other) noexcept;
        ft_pathfinding &operator=(ft_pathfinding &&other) noexcept;

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
};

#endif
