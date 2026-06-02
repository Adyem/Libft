#include "../test_internal.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/Game/game_map3d.hpp"
#include "../../Modules/Template/graph.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_world_plan_route)
{
    game_map3d grid;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, grid.initialize(3, 3, 1, 0));
    grid.set(1, 1, 0, 1);
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.plan_route(grid, 0, 0, 0, 2, 2, 0, path));
    FT_ASSERT_EQ(5u, path.size());
    return (1);
}

FT_TEST(test_astar_blocked)
{
    game_map3d grid;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, grid.initialize(2, 2, 1, 0));
    grid.set(1, 0, 0, 1);
    grid.set(0, 1, 0, 1);
    game_pathfinding finder;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, finder.initialize());
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_GAME_INVALID_MOVE, finder.astar_grid(grid, 0, 0, 0, 1, 1, 0, path));
    return (1);
}

FT_TEST(test_dijkstra_simple)
{
    ft_graph<int> graph;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, graph.initialize());
    graph.add_vertex(0);
    graph.add_vertex(1);
    graph.add_vertex(2);
    graph.add_vertex(3);
    graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    graph.add_edge(2, 3);
    game_pathfinding finder;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, finder.initialize());
    ft_vector<ft_size_t> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, finder.dijkstra_graph(graph, 0, 3, path));
    FT_ASSERT_EQ(4u, path.size());
    FT_ASSERT_EQ(0u, path[0]);
    FT_ASSERT_EQ(3u, path[3]);
    return (1);
}
