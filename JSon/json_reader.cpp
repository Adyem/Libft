#include <cctype>
#include <string>
#include "json.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"

static void skip_ws(const std::string &s, size_t &i)
{
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
        i++;
}

static std::string parse_string(const std::string &s, size_t &i)
{
    if (i >= s.size() || s[i] != '"')
        return (std::string());
    i++;
    size_t start = i;
    while (i < s.size() && s[i] != '"')
        i++;
    std::string result = s.substr(start, i - start);
    if (i < s.size() && s[i] == '"')
        i++;
    return (result);
}

static json_item *parse_items(const std::string &s, size_t &i)
{
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_ws(s, i);
    if (i >= s.size() || s[i] != '{')
        return (ft_nullptr);
    i++;
    while (i < s.size())
    {
        skip_ws(s, i);
        if (i < s.size() && s[i] == '}')
        {
            i++;
            break;
        }
        std::string key = parse_string(s, i);
        skip_ws(s, i);
        if (i >= s.size() || s[i] != ':')
            break;
        i++;
        skip_ws(s, i);
        std::string value = parse_string(s, i);
        json_item *item = json_create_item(key.c_str(), value.c_str());
        if (!item)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        if (!head)
            head = tail = item;
        else
        {
            tail->next = item;
            tail = item;
        }
        skip_ws(s, i);
        if (i < s.size() && s[i] == ',')
        {
            i++;
            continue;
        }
    }
    return (head);
}

json_group *json_read_from_file(const char *filename)
{
    char **lines = ft_open_and_read_file(filename);
    if (!lines)
        return (ft_nullptr);
    std::string content;
    for (int idx = 0; lines[idx]; ++idx)
    {
        content += lines[idx];
        cma_free(lines[idx]);
    }
    cma_free(lines);
    size_t i = 0;
    skip_ws(content, i);
    if (i >= content.size() || content[i] != '{')
        return (ft_nullptr);
    i++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (i < content.size())
    {
        skip_ws(content, i);
        if (i < content.size() && content[i] == '}')
        {
            i++;
            break;
        }
        std::string group_name = parse_string(content, i);
        skip_ws(content, i);
        if (i >= content.size() || content[i] != ':')
            break;
        i++;
        json_item *items = parse_items(content, i);
        if (!items)
        {
            json_free_groups(head);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name.c_str());
        if (!group)
        {
            json_free_items(items);
            json_free_groups(head);
            return (ft_nullptr);
        }
        group->items = items;
        if (!head)
            head = tail = group;
        else
        {
            tail->next = group;
            tail = group;
        }
        skip_ws(content, i);
        if (i < content.size() && content[i] == ',')
        {
            i++;
            continue;
        }
    }
    return (head);
}

