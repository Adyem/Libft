#include "../../Game/game_pathfinding.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_map3d.hpp"
#include "../../Template/graph.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_world_plan_route, "world plan route")
{
    ft_map3d grid(3, 3, 1, 0);
    grid.set(1, 1, 0, 1);
    ft_world world;
    ft_vector<ft_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, world.plan_route(grid, 0, 0, 0, 2, 2, 0, path));
    FT_ASSERT_EQ(5u, path.size());
    return (1);
}

FT_TEST(test_astar_blocked, "astar blocked path")
{
    ft_map3d grid(2, 2, 1, 0);
    grid.set(1, 0, 0, 1);
    grid.set(0, 1, 0, 1);
    ft_pathfinding finder;
    ft_vector<ft_path_step> path;
    FT_ASSERT_EQ(FT_ERR_GAME_INVALID_MOVE, finder.astar_grid(grid, 0, 0, 0, 1, 1, 0, path));
    return (1);
}

FT_TEST(test_dijkstra_simple, "dijkstra simple")
{
    ft_graph<int> graph;
    graph.add_vertex(0);
    graph.add_vertex(1);
    graph.add_vertex(2);
    graph.add_vertex(3);
    graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    graph.add_edge(2, 3);
    ft_pathfinding finder;
    ft_vector<size_t> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, finder.dijkstra_graph(graph, 0, 3, path));
    FT_ASSERT_EQ(4u, path.size());
    FT_ASSERT_EQ(0u, path[0]);
    FT_ASSERT_EQ(3u, path[3]);
    return (1);
}
