#ifndef GAME_PATHFINDING_HPP
# define GAME_PATHFINDING_HPP

#include "game_map3d.hpp"
#include "../Template/vector.hpp"
#include "../Template/graph.hpp"
#include "../Errno/errno.hpp"

struct ft_path_step
{
    size_t  _x;
    size_t  _y;
    size_t  _z;
};

class ft_pathfinding
{
    private:
        mutable int _error_code;

        void    set_error(int error) const noexcept;

    public:
        ft_pathfinding() noexcept;
        ~ft_pathfinding();

        int astar_grid(const ft_map3d &grid,
            size_t start_x, size_t start_y, size_t start_z,
            size_t goal_x, size_t goal_y, size_t goal_z,
            ft_vector<ft_path_step> &out_path) const noexcept;

        int dijkstra_graph(const ft_graph<int> &graph,
            size_t start_vertex, size_t goal_vertex,
            ft_vector<size_t> &out_path) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
