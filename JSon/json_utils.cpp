#include <cstring>
#include "json.hpp"
#include "../CPP_class/nullptr.hpp"

json_group *json_find_group(json_group *head, const char *name)
{
    json_group *current = head;
    while (current)
    {
        if (current->name && std::strcmp(current->name, name) == 0)
            return (current);
        current = current->next;
    }
    return (ft_nullptr);
}

json_item *json_find_item(json_group *group, const char *key)
{
    if (!group)
        return ft_nullptr;
    json_item *current = group->items;
    while (current)
    {
        if (current->key && std::strcmp(current->key, key) == 0)
            return (current);
        current = current->next;
    }
    return (ft_nullptr);
}
