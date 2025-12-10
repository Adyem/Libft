#include "../../HTML/parser.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_html_node_creation_enables_thread_safety,
        "html_create_node prepares thread safety guards")
{
    html_node *node;

    node = html_create_node("div", "text");
    FT_ASSERT(node != ft_nullptr);
    FT_ASSERT_EQ(true, html_node_is_thread_safe_enabled(node));
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_prepare_thread_safety_restores_mutex,
        "html_node_prepare_thread_safety reinstalls mutex after teardown")
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

FT_TEST(test_html_node_lock_resets_errno,
        "html_node_lock unlock cycle sets ft_errno to success")
{
    html_node *node;
    bool       lock_acquired;

    node = html_create_node("p", "paragraph");
    FT_ASSERT(node != ft_nullptr);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, html_node_lock(node, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    html_node_unlock(node, lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_teardown_clears_mutex,
        "html_node_teardown_thread_safety releases mutex")
{
    html_node *node;

    node = html_create_node("section", ft_nullptr);
    FT_ASSERT(node != ft_nullptr);
    FT_ASSERT(node->mutex != ft_nullptr);
    FT_ASSERT_EQ(true, html_node_is_thread_safe_enabled(node));
    html_node_teardown_thread_safety(node);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    FT_ASSERT(node->mutex == ft_nullptr);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_lock_after_teardown_skips_mutex,
        "html_node_lock succeeds without mutex when thread safety disabled")
{
    html_node *node;
    bool       lock_acquired;
    int        lock_result;

    node = html_create_node("header", ft_nullptr);
    FT_ASSERT(node != ft_nullptr);
    html_node_teardown_thread_safety(node);
    FT_ASSERT_EQ(false, html_node_is_thread_safe_enabled(node));
    lock_acquired = true;
    lock_result = html_node_lock(node, &lock_acquired);
    FT_ASSERT_EQ(0, lock_result);
    FT_ASSERT_EQ(false, lock_acquired);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_node_lock_null_sets_errno,
        "html_node_lock null node sets FT_ERR_INVALID_ARGUMENT")
{
    bool lock_acquired;
    int  lock_result;

    lock_acquired = true;
    ft_errno = ER_SUCCESS;
    lock_result = html_node_lock(ft_nullptr, &lock_acquired);
    FT_ASSERT_EQ(-1, lock_result);
    FT_ASSERT_EQ(false, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_html_node_unlock_null_resets_errno,
        "html_node_unlock null node resets ft_errno to success")
{
    ft_errno = FT_ERR_INVALID_OPERATION;
    html_node_unlock(ft_nullptr, true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

