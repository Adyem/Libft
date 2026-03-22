#include "../test_internal.hpp"
#include "../../HTML/html_parser.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_html_node_creation_enables_thread_safety)
{
    html_node *node;

    node = html_create_node("div", "text");
    FT_ASSERT(node != ft_nullptr);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    FT_ASSERT_EQ(0, html_node_prepare_thread_safety(node));
    FT_ASSERT_EQ(true, html_node_is_thread_safe_enabled(node));
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_prepare_thread_safety_restores_mutex)
{
    html_node *node;

    node = html_create_node("span", ft_nullptr);
    FT_ASSERT(node != ft_nullptr);
    html_node_teardown_thread_safety(node);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    FT_ASSERT_EQ(0, html_node_prepare_thread_safety(node));
    FT_ASSERT_EQ(true, html_node_is_thread_safe_enabled(node));
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_lock_resets_errno)
{
    html_node *node;
    ft_bool    lock_acquired;

    node = html_create_node("p", "paragraph");
    FT_ASSERT(node != ft_nullptr);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, html_node_lock(node, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    html_node_unlock(node, lock_acquired);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_teardown_clears_mutex)
{
    html_node *node;

    node = html_create_node("section", ft_nullptr);
    FT_ASSERT(node != ft_nullptr);
    FT_ASSERT(node->mutex == ft_nullptr);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    FT_ASSERT_EQ(0, html_node_prepare_thread_safety(node));
    FT_ASSERT(node->mutex != ft_nullptr);
    FT_ASSERT_EQ(true, html_node_is_thread_safe_enabled(node));
    html_node_teardown_thread_safety(node);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    FT_ASSERT(node->mutex == ft_nullptr);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_lock_after_teardown_skips_mutex)
{
    html_node *node;
    ft_bool    lock_acquired;
    int        lock_result;

    node = html_create_node("header", ft_nullptr);
    FT_ASSERT(node != ft_nullptr);
    html_node_teardown_thread_safety(node);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    lock_acquired = FT_TRUE;
    lock_result = html_node_lock(node, &lock_acquired);
    FT_ASSERT_EQ(0, lock_result);
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_lock_null_sets_errno)
{
    ft_bool lock_acquired;
    int  lock_result;

    lock_acquired = FT_TRUE;
    lock_result = html_node_lock(ft_nullptr, &lock_acquired);
    FT_ASSERT_EQ(3, lock_result);
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    return (1);
}

FT_TEST(test_html_node_unlock_null_resets_errno)
{
    html_node_unlock(ft_nullptr, FT_TRUE);
    return (1);
}
