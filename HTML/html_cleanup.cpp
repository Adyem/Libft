#include "html_parser.hpp"

void html_free_nodes(html_node *nodeList)
{
    while (nodeList)
    {
        html_node *nextNode = nodeList->next;
        if (nodeList->tag)
            delete[] nodeList->tag;
        if (nodeList->text)
            delete[] nodeList->text;
        html_attr *attribute = nodeList->attributes;
        while (attribute)
        {
            html_attr *nextAttr = attribute->next;
            if (attribute->key)
                delete[] attribute->key;
            if (attribute->value)
                delete[] attribute->value;
            delete attribute;
            attribute = nextAttr;
        }
        html_free_nodes(nodeList->children);
        delete nodeList;
        nodeList = nextNode;
    }
    return ;
}
