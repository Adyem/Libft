#include "pathfinding.hpp"

static size_t distance_component(size_t a, size_t b)
{
    if (a > b)
        return (a - b);
    return (b - a);
}

ft_pathfinding::ft_pathfinding() noexcept
    : _error_code(ER_SUCCESS)
{
    return ;
}

ft_pathfinding::~ft_pathfinding()
{
    return ;
}

void ft_pathfinding::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

int ft_pathfinding::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_pathfinding::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

int ft_pathfinding::astar_grid(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) const noexcept
{
    out_path.clear();
    size_t grid_width = grid.get_width();
    size_t grid_height = grid.get_height();
    size_t grid_depth = grid.get_depth();
    if (start_x >= grid_width || start_y >= grid_height || start_z >= grid_depth ||
        goal_x >= grid_width || goal_y >= grid_height || goal_z >= grid_depth)
    {
        this->set_error(GAME_INVALID_MOVE);
        return (this->_error_code);
    }
    struct node
    {
        size_t  x;
        size_t  y;
        size_t  z;
        int     g;
        int     f;
        int     parent;
    };
    ft_vector<node> nodes;
    ft_vector<size_t> open_set;
    node start;
    start.x = start_x;
    start.y = start_y;
    start.z = start_z;
    start.g = 0;
    size_t dx = distance_component(start_x, goal_x);
    size_t dy = distance_component(start_y, goal_y);
    size_t dz = distance_component(start_z, goal_z);
    start.f = dx + dy + dz;
    start.parent = -1;
    nodes.push_back(start);
    open_set.push_back(0);
    while (open_set.size() > 0)
    {
        size_t open_index = 0;
        size_t best_open = 0;
        while (open_index < open_set.size())
        {
            size_t current_open = open_set[open_index];
            size_t best_candidate = open_set[best_open];
            if (nodes[current_open].f < nodes[best_candidate].f)
                best_open = open_index;
            ++open_index;
        }
        size_t current_index = open_set[best_open];
        ft_path_step current_step;
        current_step._x = nodes[current_index].x;
        current_step._y = nodes[current_index].y;
        current_step._z = nodes[current_index].z;
        if (current_step._x == goal_x && current_step._y == goal_y && current_step._z == goal_z)
        {
            ft_vector<ft_path_step> reverse_path;
            size_t trace_index = current_index;
            while (true)
            {
                ft_path_step step;
                step._x = nodes[trace_index].x;
                step._y = nodes[trace_index].y;
                step._z = nodes[trace_index].z;
                reverse_path.push_back(step);
                if (nodes[trace_index].parent == -1)
                    break;
                trace_index = static_cast<size_t>(nodes[trace_index].parent);
            }
            size_t rev_index = reverse_path.size();
            while (rev_index > 0)
            {
                --rev_index;
                out_path.push_back(reverse_path[rev_index]);
            }
            this->set_error(ER_SUCCESS);
            return (ER_SUCCESS);
        }
        open_set.erase(open_set.begin() + best_open);
        int dir_x[6] = {1, -1, 0, 0, 0, 0};
        int dir_y[6] = {0, 0, 1, -1, 0, 0};
        int dir_z[6] = {0, 0, 0, 0, 1, -1};
        size_t neighbor_index = 0;
        while (neighbor_index < 6)
        {
            size_t neighbor_x = nodes[current_index].x;
            if (dir_x[neighbor_index] > 0)
                neighbor_x = neighbor_x + 1;
            else if (dir_x[neighbor_index] < 0)
                neighbor_x = neighbor_x - 1;
            size_t neighbor_y = nodes[current_index].y;
            if (dir_y[neighbor_index] > 0)
                neighbor_y = neighbor_y + 1;
            else if (dir_y[neighbor_index] < 0)
                neighbor_y = neighbor_y - 1;
            size_t neighbor_z = nodes[current_index].z;
            if (dir_z[neighbor_index] > 0)
                neighbor_z = neighbor_z + 1;
            else if (dir_z[neighbor_index] < 0)
                neighbor_z = neighbor_z - 1;
            if (neighbor_x < grid_width && neighbor_y < grid_height && neighbor_z < grid_depth &&
                grid.get(neighbor_x, neighbor_y, neighbor_z) == 0)
            {
                size_t search_index = 0;
                bool found = false;
                while (search_index < nodes.size())
                {
                    if (nodes[search_index].x == neighbor_x &&
                        nodes[search_index].y == neighbor_y &&
                        nodes[search_index].z == neighbor_z)
                    {
                        found = true;
                        break;
                    }
                    ++search_index;
                }
                int tentative_g = nodes[current_index].g + 1;
                if (found)
                {
                    if (tentative_g < nodes[search_index].g)
                    {
                        nodes[search_index].g = tentative_g;
                        size_t ndx = distance_component(neighbor_x, goal_x);
                        size_t ndy = distance_component(neighbor_y, goal_y);
                        size_t ndz = distance_component(neighbor_z, goal_z);
                        nodes[search_index].f = tentative_g + ndx + ndy + ndz;
                        nodes[search_index].parent = static_cast<int>(current_index);
                        size_t check_index = 0;
                        bool in_open = false;
                        while (check_index < open_set.size())
                        {
                            if (open_set[check_index] == search_index)
                                in_open = true;
                            ++check_index;
                        }
                        if (!in_open)
                            open_set.push_back(search_index);
                    }
                }
                else
                {
                    node new_node;
                    new_node.x = neighbor_x;
                    new_node.y = neighbor_y;
                    new_node.z = neighbor_z;
                    new_node.g = tentative_g;
                    size_t ndx = distance_component(neighbor_x, goal_x);
                    size_t ndy = distance_component(neighbor_y, goal_y);
                    size_t ndz = distance_component(neighbor_z, goal_z);
                    new_node.f = tentative_g + ndx + ndy + ndz;
                    new_node.parent = static_cast<int>(current_index);
                    nodes.push_back(new_node);
                    open_set.push_back(nodes.size() - 1);
                }
            }
            ++neighbor_index;
        }
    }
    this->set_error(GAME_INVALID_MOVE);
    return (this->_error_code);
}

int ft_pathfinding::dijkstra_graph(const ft_graph<int> &graph,
    size_t start_vertex, size_t goal_vertex,
    ft_vector<size_t> &out_path) const noexcept
{
    out_path.clear();
    size_t graph_size = graph.size();
    if (start_vertex >= graph_size || goal_vertex >= graph_size)
    {
        this->set_error(GAME_INVALID_MOVE);
        return (this->_error_code);
    }
    ft_vector<int> distance;
    ft_vector<int> previous;
    distance.resize(graph_size, -1);
    previous.resize(graph_size, -1);
    ft_vector<size_t> queue;
    queue.push_back(start_vertex);
    distance[start_vertex] = 0;
    while (queue.size() > 0)
    {
        size_t queue_index = 0;
        size_t best_queue = 0;
        while (queue_index < queue.size())
        {
            if (distance[queue[queue_index]] < distance[queue[best_queue]])
                best_queue = queue_index;
            ++queue_index;
        }
        size_t current = queue[best_queue];
        queue.erase(queue.begin() + best_queue);
        if (current == goal_vertex)
            break;
        ft_vector<size_t> neighbors;
        graph.neighbors(current, neighbors);
        size_t neighbor_index = 0;
        while (neighbor_index < neighbors.size())
        {
            size_t neighbor = neighbors[neighbor_index];
            int alt = distance[current] + 1;
            if (distance[neighbor] == -1 || alt < distance[neighbor])
            {
                distance[neighbor] = alt;
                previous[neighbor] = static_cast<int>(current);
                size_t search_index = 0;
                bool found = false;
                while (search_index < queue.size())
                {
                    if (queue[search_index] == neighbor)
                        found = true;
                    ++search_index;
                }
                if (!found)
                    queue.push_back(neighbor);
            }
            ++neighbor_index;
        }
    }
    if (distance[goal_vertex] == -1)
    {
        this->set_error(GAME_INVALID_MOVE);
        return (this->_error_code);
    }
    ft_vector<size_t> reverse_path;
    size_t vertex = goal_vertex;
    while (true)
    {
        reverse_path.push_back(vertex);
        if (vertex == start_vertex)
            break;
        vertex = static_cast<size_t>(previous[vertex]);
    }
    size_t rev_index = reverse_path.size();
    while (rev_index > 0)
    {
        --rev_index;
        out_path.push_back(reverse_path[rev_index]);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}
