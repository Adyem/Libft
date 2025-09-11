#include "../XML/xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include <cstring>

int test_xml_parse_simple(void)
{
    const char *xml = "<root><child>value</child></root>";
    xml_document doc;
    if (doc.load_from_string(xml) != ER_SUCCESS)
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

