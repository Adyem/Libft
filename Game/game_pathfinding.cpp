#include "game_pathfinding.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static size_t distance_component(size_t left_value, size_t right_value)
{
    if (left_value > right_value)
        return (left_value - right_value);
    return (right_value - left_value);
}

void ft_path_step::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_path_step lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_path_step::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_path_step::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_path_step::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_path_step::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

ft_path_step::ft_path_step() noexcept
    : _x(0), _y(0), _z(0), _mutex(ft_nullptr),
      _initialized_state(ft_path_step::_state_uninitialized)
{
    return ;
}

ft_path_step::~ft_path_step() noexcept
{
    if (this->_initialized_state != ft_path_step::_state_initialized)
        return ;
    (void)this->destroy();
    return ;
}

int ft_path_step::initialize() noexcept
{
    if (this->_initialized_state == ft_path_step::_state_initialized)
    {
        this->abort_lifecycle_error("ft_path_step::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_path_step::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_path_step::initialize(const ft_path_step &other) noexcept
{
    if (other._initialized_state != ft_path_step::_state_initialized)
    {
        other.abort_lifecycle_error("ft_path_step::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_path_step::_state_initialized)
    {
        this->abort_lifecycle_error("ft_path_step::initialize(copy)",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_initialized_state = ft_path_step::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_path_step::initialize(ft_path_step &&other) noexcept
{
    if (other._initialized_state != ft_path_step::_state_initialized)
    {
        other.abort_lifecycle_error("ft_path_step::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_path_step::_state_initialized)
    {
        this->abort_lifecycle_error("ft_path_step::initialize(move)",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_initialized_state = ft_path_step::_state_initialized;
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other._initialized_state = ft_path_step::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_path_step::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_path_step::_state_initialized)
    {
        this->_initialized_state = ft_path_step::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_x = 0;
    this->_y = 0;
    this->_z = 0;
    this->_initialized_state = ft_path_step::_state_destroyed;
    return (disable_error);
}

int ft_path_step::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_path_step::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_path_step::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_path_step::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_path_step::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_path_step::set_coordinates(size_t x, size_t y, size_t z) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_path_step::set_coordinates");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_x = x;
    this->_y = y;
    this->_z = z;
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_path_step::set_x(size_t x) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_path_step::set_x");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_x = x;
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_path_step::set_y(size_t y) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_path_step::set_y");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_y = y;
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_path_step::set_z(size_t z) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_path_step::set_z");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_z = z;
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

size_t ft_path_step::get_x() const noexcept
{
    this->abort_if_not_initialized("ft_path_step::get_x");
    return (this->_x);
}

size_t ft_path_step::get_y() const noexcept
{
    this->abort_if_not_initialized("ft_path_step::get_y");
    return (this->_y);
}

size_t ft_path_step::get_z() const noexcept
{
    this->abort_if_not_initialized("ft_path_step::get_z");
    return (this->_z);
}

int ft_path_step::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_path_step::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}

void ft_path_step::reset_system_error() const noexcept
{
    return ;
}

void ft_pathfinding::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_pathfinding lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_pathfinding::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_pathfinding::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_pathfinding::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_pathfinding::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

ft_pathfinding::ft_pathfinding() noexcept
    : _current_path(), _needs_replan(false), _mutex(ft_nullptr),
      _initialized_state(ft_pathfinding::_state_uninitialized)
{
    return ;
}

ft_pathfinding::~ft_pathfinding()
{
    if (this->_initialized_state != ft_pathfinding::_state_initialized)
        return ;
    (void)this->destroy();
    return ;
}

int ft_pathfinding::initialize() noexcept
{
    if (this->_initialized_state == ft_pathfinding::_state_initialized)
    {
        this->abort_lifecycle_error("ft_pathfinding::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_needs_replan = false;
    this->_current_path.clear();
    this->_initialized_state = ft_pathfinding::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_pathfinding::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_pathfinding::_state_initialized)
    {
        this->_initialized_state = ft_pathfinding::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_current_path.clear();
    this->_needs_replan = false;
    this->_initialized_state = ft_pathfinding::_state_destroyed;
    return (disable_error);
}

int ft_pathfinding::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_pathfinding::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_pathfinding::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_pathfinding::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_pathfinding::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_pathfinding::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_pathfinding::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_pathfinding::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_pathfinding::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_pathfinding::update_obstacle(size_t x, size_t y, size_t z,
    int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    size_t index;

    (void)value;
    this->abort_if_not_initialized("ft_pathfinding::update_obstacle");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    index = 0;
    while (index < this->_current_path.size())
    {
        if (this->_current_path[index].get_x() == x
            && this->_current_path[index].get_y() == y
            && this->_current_path[index].get_z() == z)
        {
            this->_needs_replan = true;
            (void)this->unlock_internal(lock_acquired);
            return ;
        }
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_pathfinding::recalculate_path(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) noexcept
{
    bool lock_acquired;
    int lock_error;
    int result;
    size_t index;

    this->abort_if_not_initialized("ft_pathfinding::recalculate_path");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_needs_replan == false && this->_current_path.size() > 0)
    {
        out_path.clear();
        index = 0;
        while (index < this->_current_path.size())
        {
            out_path.push_back(this->_current_path[index]);
            index += 1;
        }
        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_SUCCESS);
    }
    result = this->astar_grid(grid, start_x, start_y, start_z,
            goal_x, goal_y, goal_z, out_path);
    if (result == FT_ERR_SUCCESS)
    {
        this->_current_path.clear();
        index = 0;
        while (index < out_path.size())
        {
            this->_current_path.push_back(out_path[index]);
            index += 1;
        }
        this->_needs_replan = false;
    }
    else
        this->_needs_replan = true;
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

int ft_pathfinding::astar_grid(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) const noexcept
{
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

    if (start_x >= grid.get_width() || start_y >= grid.get_height()
        || start_z >= grid.get_depth() || goal_x >= grid.get_width()
        || goal_y >= grid.get_height() || goal_z >= grid.get_depth())
        return (FT_ERR_GAME_INVALID_MOVE);

    out_path.clear();

    node start_node;
    start_node.x = start_x;
    start_node.y = start_y;
    start_node.z = start_z;
    start_node.g = 0;
    start_node.f = static_cast<int>(distance_component(start_x, goal_x)
            + distance_component(start_y, goal_y)
            + distance_component(start_z, goal_z));
    start_node.parent = -1;
    nodes.push_back(start_node);
    open_set.push_back(0);

    while (open_set.size() > 0)
    {
        size_t best_open;
        size_t open_index;
        size_t current_index;

        best_open = 0;
        open_index = 0;
        while (open_index < open_set.size())
        {
            if (nodes[open_set[open_index]].f < nodes[open_set[best_open]].f)
                best_open = open_index;
            open_index += 1;
        }
        current_index = open_set[best_open];

        if (nodes[current_index].x == goal_x && nodes[current_index].y == goal_y
            && nodes[current_index].z == goal_z)
        {
            ft_vector<ft_path_step> reverse_path;
            size_t trace_index;
            size_t reverse_index;

            trace_index = current_index;
            while (true)
            {
                ft_path_step step;

                (void)step.initialize();
                (void)step.set_coordinates(nodes[trace_index].x,
                    nodes[trace_index].y, nodes[trace_index].z);
                reverse_path.push_back(step);
                if (nodes[trace_index].parent == -1)
                    break ;
                trace_index = static_cast<size_t>(nodes[trace_index].parent);
            }
            reverse_index = reverse_path.size();
            while (reverse_index > 0)
            {
                reverse_index -= 1;
                out_path.push_back(reverse_path[reverse_index]);
            }
            return (FT_ERR_SUCCESS);
        }

        open_set.erase(open_set.begin() + best_open);

        int direction_x[6] = {1, -1, 0, 0, 0, 0};
        int direction_y[6] = {0, 0, 1, -1, 0, 0};
        int direction_z[6] = {0, 0, 0, 0, 1, -1};
        size_t neighbor_index;

        neighbor_index = 0;
        while (neighbor_index < 6)
        {
            size_t neighbor_x;
            size_t neighbor_y;
            size_t neighbor_z;

            neighbor_x = nodes[current_index].x;
            neighbor_y = nodes[current_index].y;
            neighbor_z = nodes[current_index].z;
            if (direction_x[neighbor_index] > 0)
                neighbor_x += 1;
            else if (direction_x[neighbor_index] < 0)
                neighbor_x -= 1;
            if (direction_y[neighbor_index] > 0)
                neighbor_y += 1;
            else if (direction_y[neighbor_index] < 0)
                neighbor_y -= 1;
            if (direction_z[neighbor_index] > 0)
                neighbor_z += 1;
            else if (direction_z[neighbor_index] < 0)
                neighbor_z -= 1;
            if (neighbor_x < grid.get_width() && neighbor_y < grid.get_height()
                && neighbor_z < grid.get_depth()
                && grid.get(neighbor_x, neighbor_y, neighbor_z) == 0)
            {
                bool found;
                size_t search_index;
                int tentative_g;

                found = false;
                search_index = 0;
                while (search_index < nodes.size())
                {
                    if (nodes[search_index].x == neighbor_x
                        && nodes[search_index].y == neighbor_y
                        && nodes[search_index].z == neighbor_z)
                    {
                        found = true;
                        break ;
                    }
                    search_index += 1;
                }
                tentative_g = nodes[current_index].g + 1;
                if (found)
                {
                    if (tentative_g < nodes[search_index].g)
                    {
                        nodes[search_index].g = tentative_g;
                        nodes[search_index].f = tentative_g + static_cast<int>(
                                distance_component(neighbor_x, goal_x)
                                + distance_component(neighbor_y, goal_y)
                                + distance_component(neighbor_z, goal_z));
                        nodes[search_index].parent = static_cast<int>(current_index);
                    }
                }
                else
                {
                    node new_node;

                    new_node.x = neighbor_x;
                    new_node.y = neighbor_y;
                    new_node.z = neighbor_z;
                    new_node.g = tentative_g;
                    new_node.f = tentative_g + static_cast<int>(
                            distance_component(neighbor_x, goal_x)
                            + distance_component(neighbor_y, goal_y)
                            + distance_component(neighbor_z, goal_z));
                    new_node.parent = static_cast<int>(current_index);
                    nodes.push_back(new_node);
                    open_set.push_back(nodes.size() - 1);
                }
            }
            neighbor_index += 1;
        }
    }
    return (FT_ERR_GAME_INVALID_MOVE);
}

int ft_pathfinding::dijkstra_graph(const ft_graph<int> &graph,
    size_t start_vertex, size_t goal_vertex,
    ft_vector<size_t> &out_path) const noexcept
{
    ft_vector<int> distance;
    ft_vector<int> previous;
    ft_vector<size_t> queue;

    out_path.clear();
    if (start_vertex >= graph.size() || goal_vertex >= graph.size())
        return (FT_ERR_GAME_INVALID_MOVE);

    distance.resize(graph.size(), -1);
    previous.resize(graph.size(), -1);
    distance[start_vertex] = 0;
    queue.push_back(start_vertex);

    while (queue.size() > 0)
    {
        size_t best_queue;
        size_t queue_index;
        size_t current;

        best_queue = 0;
        queue_index = 0;
        while (queue_index < queue.size())
        {
            if (distance[queue[queue_index]] < distance[queue[best_queue]])
                best_queue = queue_index;
            queue_index += 1;
        }
        current = queue[best_queue];
        queue.erase(queue.begin() + best_queue);
        if (current == goal_vertex)
            break ;

        ft_vector<size_t> neighbors;
        size_t neighbor_index;

        graph.neighbors(current, neighbors);
        neighbor_index = 0;
        while (neighbor_index < neighbors.size())
        {
            size_t neighbor;
            int alternative_distance;

            neighbor = neighbors[neighbor_index];
            alternative_distance = distance[current] + 1;
            if (distance[neighbor] == -1 || alternative_distance < distance[neighbor])
            {
                distance[neighbor] = alternative_distance;
                previous[neighbor] = static_cast<int>(current);
                queue.push_back(neighbor);
            }
            neighbor_index += 1;
        }
    }

    if (distance[goal_vertex] == -1)
        return (FT_ERR_GAME_INVALID_MOVE);

    ft_vector<size_t> reverse_path;
    size_t vertex;

    vertex = goal_vertex;
    while (true)
    {
        reverse_path.push_back(vertex);
        if (vertex == start_vertex)
            break ;
        vertex = static_cast<size_t>(previous[vertex]);
    }
    while (reverse_path.size() > 0)
    {
        out_path.push_back(reverse_path[reverse_path.size() - 1]);
        reverse_path.pop_back();
    }
    return (FT_ERR_SUCCESS);
}

int ft_pathfinding::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_pathfinding::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_pathfinding::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
