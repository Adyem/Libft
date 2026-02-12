#include "../test_internal.hpp"
#include "../../Template/graph.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_graph_move_constructor_clears_error_state,
        "ft_graph move constructor clears source error and keeps graph usable")
{
    ft_graph<int> source_graph;
    ft_vector<size_t> neighbor_vector;

    source_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source_graph.get_error());

    ft_graph<int> moved_graph(ft_move(source_graph));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_graph.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.get_error());
    moved_graph.add_vertex(1);
    moved_graph.add_vertex(2);
    moved_graph.add_edge(0, 1);
    neighbor_vector.clear();
    moved_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    source_graph.add_vertex(9);
    FT_ASSERT_EQ(1u, source_graph.size());
    FT_ASSERT_EQ(0, source_graph.get_error());
    return (1);
}

FT_TEST(test_ft_graph_move_assignment_clears_error_state,
        "ft_graph move assignment clears source error and allows reuse")
{
    ft_graph<int> destination_graph;
    ft_graph<int> source_graph;
    ft_vector<size_t> neighbor_vector;

    destination_graph.add_vertex(7);
    destination_graph.neighbors(0, neighbor_vector);
    neighbor_vector.clear();

    source_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source_graph.get_error());

    destination_graph = ft_move(source_graph);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.get_error());
    destination_graph.add_vertex(3);
    destination_graph.add_vertex(4);
    destination_graph.add_edge(0, 1);
    neighbor_vector.clear();
    destination_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    source_graph.add_vertex(11);
    source_graph.add_vertex(12);
    source_graph.add_edge(0, 1);
    neighbor_vector.clear();
    source_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    FT_ASSERT_EQ(0, destination_graph.get_error());
    FT_ASSERT_EQ(0, source_graph.get_error());
    return (1);
}
