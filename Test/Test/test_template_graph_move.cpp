#include "../../Template/graph.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_graph_move_constructor_recreates_mutex,
        "ft_graph move constructor rebuilds thread safety and keeps topology")
{
    ft_graph<int> source_graph;
    ft_vector<size_t> neighbor_vector;

    source_graph.add_vertex(10);
    source_graph.add_vertex(20);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    ft_graph<int> moved_graph(ft_move(source_graph));

    FT_ASSERT(moved_graph.is_thread_safe());
    FT_ASSERT_EQ(false, source_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, moved_graph.size());
    moved_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    FT_ASSERT_EQ(0, moved_graph.get_error());
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_graph_move_assignment_resets_source_mutex,
        "ft_graph move assignment recreates mutex and transfers edges")
{
    ft_graph<int> destination_graph;
    ft_graph<int> source_graph;
    ft_vector<size_t> neighbor_vector;

    destination_graph.add_vertex(1);
    destination_graph.add_vertex(2);
    destination_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, destination_graph.enable_thread_safety());
    FT_ASSERT(destination_graph.is_thread_safe());

    source_graph.add_vertex(3);
    source_graph.add_vertex(4);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    destination_graph = ft_move(source_graph);

    FT_ASSERT(destination_graph.is_thread_safe());
    FT_ASSERT_EQ(false, source_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_graph.size());
    destination_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    FT_ASSERT_EQ(0, destination_graph.get_error());
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_graph_move_allows_reuse_after_transfer,
        "ft_graph move keeps destination safe and lets source rebuild state")
{
    ft_graph<int> source_graph;
    ft_graph<int> destination_graph;
    ft_vector<size_t> neighbor_vector;

    source_graph.add_vertex(30);
    source_graph.add_vertex(40);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    destination_graph = ft_move(source_graph);

    FT_ASSERT(destination_graph.is_thread_safe());
    FT_ASSERT_EQ(false, source_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_graph.size());
    destination_graph.neighbors(1, neighbor_vector);
    FT_ASSERT(neighbor_vector.empty());
    FT_ASSERT_EQ(0, destination_graph.get_error());

    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());
    source_graph.add_vertex(99);
    FT_ASSERT_EQ(1u, source_graph.size());
    source_graph.add_edge(0, 0);
    neighbor_vector.clear();
    source_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(0u, neighbor_vector[0]);
    FT_ASSERT_EQ(0, source_graph.get_error());
    return (1);
}
