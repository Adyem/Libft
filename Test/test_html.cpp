#include "../HTML/html_parser.hpp"
#include "../CMA/CMA.hpp"
#include <cstring>

int test_html_create_node(void)
{
    html_node *node = html_create_node("div", "content");
    if (!node)
        return 0;
    int ok = node->tag && std::strcmp(node->tag, "div") == 0 &&
             node->text && std::strcmp(node->text, "content") == 0;
    html_free_nodes(node);
    return ok;
}

int test_html_find_by_tag(void)
{
    html_node *root = html_create_node("div", NULL);
    html_node *child = html_create_node("span", NULL);
    html_add_child(root, child);
    html_node *found = html_find_by_tag(root, "span");
    int ok = (found == child);
    html_free_nodes(root);
    return ok;
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
    return ok;
}

int test_html_find_by_attr(void)
{
    html_node *root = html_create_node("div", NULL);
    html_node *child = html_create_node("p", NULL);
    html_attr *attr = html_create_attr("id", "main");
    html_add_attr(child, attr);
    html_add_child(root, child);
    html_node *found = html_find_by_attr(root, "id", "main");
    int ok = (found == child);
    html_free_nodes(root);
    return ok;
}

