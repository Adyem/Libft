#include "../test_internal.hpp"
#include "../../XML/xml.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

int test_xml_parse_simple(void)
{
    const char *xml = "<root><child>value</child></root>";
    xml_document doc;
    int initialize_result;

    initialize_result = doc.initialize();
    if (initialize_result != FT_ERR_SUCCESS)
        return (0);
    if (doc.load_from_string(xml) != FT_ERR_SUCCESS)
        return (0);
    xml_node *root = doc.get_root();
    if (!root)
        return (0);
    if (root->children.size() != 1)
        return (0);
    xml_node *child = root->children[0];
    int ok = child && child->text && std::strcmp(child->text, "value") == 0;
    char *written = doc.write_to_string();
    if (!written)
        return (0);
    int ok2 = std::strcmp(written, "<root><child>value</child></root>\n") == 0;
    cma_free(written);
    return (ok && ok2);
}

int test_xml_propagates_child_allocation_failure(void)
{
    const char *xml = "<root><a/><b/><c/></root>";
    xml_document doc;
    int initialize_result;

    initialize_result = doc.initialize();
    if (initialize_result != FT_ERR_SUCCESS)
        return (0);
    cma_set_alloc_limit(16);
    int load_result = doc.load_from_string(xml);
    cma_set_alloc_limit(0);
    if (load_result != FT_ERR_NO_MEMORY)
        return (0);
    if (doc.get_root() != ft_nullptr)
        return (0);
    return (1);
}
