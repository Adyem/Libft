#include "../HTML/parser.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
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

