#include "../test_internal.hpp"
#include "../../Modules/Template/graph.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int graph_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_graph_get_error_returned = FT_FALSE;
static int32_t g_graph_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_graph_get_error_str_returned = FT_FALSE;
static const char *g_graph_get_error_str_result = ft_nullptr;

static void graph_get_error_uninitialised_operation(void)
{
    ft_graph<int> graph_value;

    g_graph_get_error_result = graph_value.get_error();
    g_graph_get_error_returned = FT_TRUE;
    return ;
}

static void graph_get_error_str_uninitialised_operation(void)
{
    ft_graph<int> graph_value;

    g_graph_get_error_str_result = graph_value.get_error_str();
    g_graph_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_graph_move_constructor_recreates_mutex)
{
    ft_graph<int> source_graph;
    ft_vector<ft_size_t> neighbor_vector;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.initialize());
    source_graph.add_vertex(10);
    source_graph.add_vertex(20);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    ft_graph<int> moved_graph;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_graph.move(source_graph));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_graph.get_error());
    FT_ASSERT(moved_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, moved_graph.size());
    moved_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    FT_ASSERT_EQ(false, source_graph.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.destroy());
    return (1);
}

FT_TEST(test_ft_graph_move_assignment_resets_source_mutex)
{
    ft_graph<int> destination_graph;
    ft_graph<int> source_graph;
    ft_vector<ft_size_t> neighbor_vector;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.initialize());
    destination_graph.add_vertex(1);
    destination_graph.add_vertex(2);
    destination_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, destination_graph.enable_thread_safety());
    FT_ASSERT(destination_graph.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    source_graph.add_vertex(3);
    source_graph.add_vertex(4);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.move(source_graph));
    FT_ASSERT(destination_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_graph.size());
    destination_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(1u, neighbor_vector[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    FT_ASSERT_EQ(false, source_graph.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.destroy());
    return (1);
}

FT_TEST(test_ft_graph_move_allows_reuse_after_transfer)
{
    ft_graph<int> source_graph;
    ft_graph<int> destination_graph;
    ft_vector<ft_size_t> neighbor_vector;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.initialize());
    source_graph.add_vertex(30);
    source_graph.add_vertex(40);
    source_graph.add_edge(0, 1);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.move(source_graph));
    FT_ASSERT(destination_graph.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_graph.size());
    destination_graph.neighbors(1, neighbor_vector);
    FT_ASSERT(neighbor_vector.empty());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.initialize());
    source_graph.add_vertex(99);
    FT_ASSERT_EQ(0, source_graph.enable_thread_safety());
    FT_ASSERT(source_graph.is_thread_safe());
    FT_ASSERT_EQ(1u, source_graph.size());
    source_graph.add_edge(0, 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.initialize());
    source_graph.neighbors(0, neighbor_vector);
    FT_ASSERT_EQ(1u, neighbor_vector.size());
    FT_ASSERT_EQ(0u, neighbor_vector[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_graph.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, neighbor_vector.destroy());
    return (1);
}

FT_TEST(test_ft_graph_error_queries_follow_lifecycle_contract)
{
    ft_graph<int> graph_value;

    g_graph_get_error_returned = FT_FALSE;
    g_graph_get_error_result = FT_ERR_SUCCESS;
    g_graph_get_error_str_returned = FT_FALSE;
    g_graph_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, graph_expect_sigabrt(
        graph_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_graph_get_error_returned);
    FT_ASSERT_EQ(1, graph_expect_sigabrt(
        graph_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_graph_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, graph_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, graph_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, graph_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, graph_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(graph_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
