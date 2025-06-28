#include <cstring>
#include "html_parser.hpp"
#include "../CPP_class/nullptr.hpp"

html_node *html_find_by_tag(html_node *nodeList, const char *tagName)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        if (currentNode->tag && std::strcmp(currentNode->tag, tagName) == 0)
            return (currentNode);
        html_node *found = html_find_by_tag(currentNode->children, tagName);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}
