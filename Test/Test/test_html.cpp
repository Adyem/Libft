#include "../../HTML/parser.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

int test_html_create_node(void)
{
    html_node *node = html_create_node("div", "content");
    if (!node)
        return (0);
    int ok = node->tag && std::strcmp(node->tag, "div") == 0 &&
             node->text && std::strcmp(node->text, "content") == 0;
    html_free_nodes(node);
    return (ok);
}

int test_html_find_by_tag(void)
{
    html_node *root = html_create_node("div", ft_nullptr);
    html_node *child = html_create_node("span", ft_nullptr);
    html_add_child(root, child);
    html_node *found = html_find_by_tag(root, "span");
    int ok = (found == child);
    html_free_nodes(root);
    return (ok);
}

int test_html_write_to_string(void)
{
    html_node *node = html_create_node("div", "Hello");
    char *result = html_write_to_string(node);
    const char *expected = "<div>Hello</div>\n";
    int ok = result && std::strcmp(result, expected) == 0;
    if (result)
        cma_free(result);
    html_free_nodes(node);
    return (ok);
}

FT_TEST(test_html_write_to_string_allocation_failure_sets_errno, "html_write_to_string sets errno on allocation failures")
{
    html_node *node = html_create_node("div", "Hello");
    FT_ASSERT(node != ft_nullptr);
    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    char *result = html_write_to_string(node);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    html_free_nodes(node);
    return (1);
}

FT_TEST(test_html_write_to_string_success_clears_errno, "html_write_to_string clears errno after successful serialization")
{
    html_node *node = html_create_node("div", "Hello");
    FT_ASSERT(node != ft_nullptr);
    ft_errno = FT_EINVAL;
    char *result = html_write_to_string(node);
    FT_ASSERT(result != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(result, "<div>Hello</div>\n"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(result);
    html_free_nodes(node);
    return (1);
}

int test_html_find_by_attr(void)
{
    html_node *root = html_create_node("div", ft_nullptr);
    html_node *child = html_create_node("p", ft_nullptr);
    html_attr *attr = html_create_attr("id", "main");
    html_add_attr(child, attr);
    html_add_child(root, child);
    html_node *found = html_find_by_attr(root, "id", "main");
    int ok = (found == child);
    html_free_nodes(root);
    return (ok);
}

int test_html_find_by_selector(void)
{
    html_node *root = html_create_node("div", ft_nullptr);
    html_node *child_id = html_create_node("p", ft_nullptr);
    html_attr *id_attr = html_create_attr("id", "main");
    html_add_attr(child_id, id_attr);
    html_add_child(root, child_id);
    html_node *child_class = html_create_node("span", ft_nullptr);
    html_attr *class_attr = html_create_attr("class", "highlight");
    html_add_attr(child_class, class_attr);
    html_add_child(root, child_class);
    html_node *found_id = html_find_by_selector(root, "#main");
    html_node *found_class = html_find_by_selector(root, ".highlight");
    int ok = (found_id == child_id) && (found_class == child_class);
    html_free_nodes(root);
    return (ok);
}

int test_html_query_selector(void)
{
    html_node *root = html_create_node("div", ft_nullptr);
    html_node *child_tag = html_create_node("p", ft_nullptr);
    html_node *child_class = html_create_node("span", ft_nullptr);
    html_attr *class_attr = html_create_attr("class", "note");
    html_add_attr(child_class, class_attr);
    html_node *child_id = html_create_node("a", ft_nullptr);
    html_attr *id_attr = html_create_attr("id", "link");
    html_add_attr(child_id, id_attr);
    html_add_child(root, child_tag);
    html_add_child(root, child_class);
    html_add_child(root, child_id);
    html_node *found_tag = html_query_selector(root, "p");
    html_node *found_class = html_query_selector(root, ".note");
    html_node *found_id = html_query_selector(root, "#link");
    int is_ok = (found_tag == child_tag) && (found_class == child_class) && (found_id == child_id);
    html_free_nodes(root);
    return (is_ok);
}

