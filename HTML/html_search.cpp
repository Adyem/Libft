#include "html_parser.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"

html_node *html_find_by_tag(html_node *nodeList, const char *tagName)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        if (currentNode->tag && ft_strcmp(currentNode->tag, tagName) == 0)
            return (currentNode);
        html_node *found = html_find_by_tag(currentNode->children, tagName);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}

html_node *html_find_by_attr(html_node *nodeList, const char *key, const char *value)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        html_attr *attribute = currentNode->attributes;
        while (attribute)
        {
            if (attribute->key && ft_strcmp(attribute->key, key) == 0)
            {
                if (!value || (attribute->value && ft_strcmp(attribute->value, value) == 0))
                    return (currentNode);
            }
            attribute = attribute->next;
        }
        html_node *found = html_find_by_attr(currentNode->children, key, value);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}
