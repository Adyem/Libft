#include <cctype>
#include <string>
#include "json.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"

static void skip_ws(const char *s, size_t &i)
{
    while (s[i] && std::isspace(static_cast<unsigned char>(s[i])))
        i++;
    return ;
}

static char *parse_string(const std::string &s, size_t &i)
{
    if (i >= s.size() || s[i] != '"')
        return (ft_nullptr);
    i++;
    size_t start = i;
    while (i < s.size() && s[i] != '"')
        i++;
    char *result = cma_substr(s.c_str(), start, i - start);
    if (i < s.size() && s[i] == '"')
        i++;
    return (result);
}

static char *parse_number(const std::string &s, size_t &i)
{
    size_t start = i;
    if (i < s.size() && (s[i] == '-' || s[i] == '+'))
        i++;
    bool has_digits = false;
    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
    {
        i++;
        has_digits = true;
    }
    if (i < s.size() && s[i] == '.')
    {
        i++;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
            i++;
    }
    if (i < s.size() && (s[i] == 'e' || s[i] == 'E'))
    {
        i++;
        if (i < s.size() && (s[i] == '-' || s[i] == '+'))
            i++;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
            i++;
    }
    if (!has_digits)
        return (ft_nullptr);
    return (cma_substr(s.c_str(), start, i - start));
}

static char *parse_value(const std::string &s, size_t &i)
{
    skip_ws(s.c_str(), i);
    if (i >= s.size())
        return (ft_nullptr);
    if (s[i] == '"')
        return (parse_string(s, i));
    if (s.compare(i, 4, "true") == 0)
    {
        i += 4;
        return (cma_strdup("true"));
    }
    if (s.compare(i, 5, "false") == 0)
    {
        i += 5;
        return (cma_strdup("false"));
    }
    if (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '-' || s[i] == '+')
        return (parse_number(s, i));
    return (ft_nullptr);
}

static json_item *parse_items(const char *s, size_t &i)
{
    std::string str(s);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_ws(s, i);
    if (i >= str.size() || str[i] != '{')
        return (ft_nullptr);
    i++;
    while (i < str.size())
    {
        skip_ws(s, i);
        if (i < str.size() && str[i] == '}')
        {
            i++;
            break;
        }
        char *key = parse_string(str, i);
        if (!key)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        skip_ws(s, i);
        if (i >= str.size() || str[i] != ':')
        {
            cma_free(key);
            break;
        }
        i++;
        skip_ws(s, i);
        char *value = parse_value(str, i);
        if (!value)
        {
            cma_free(key);
            json_free_items(head);
            return (ft_nullptr);
        }
        json_item *item = json_create_item(key, value);
        cma_free(key);
        cma_free(value);
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
        if (i < str.size() && str[i] == ',')
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
    skip_ws(content.c_str(), i);
    if (i >= content.size() || content[i] != '{')
        return (ft_nullptr);
    i++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (i < content.size())
    {
        skip_ws(content.c_str(), i);
        if (i < content.size() && content[i] == '}')
        {
            i++;
            break;
        }
        char *group_name = parse_string(content, i);
        if (!group_name)
        {
            json_free_groups(head);
            return (ft_nullptr);
        }
        skip_ws(content.c_str(), i);
        if (i >= content.size() || content[i] != ':')
        {
            cma_free(group_name);
            break;
        }
        i++;
        json_item *items = parse_items(content.c_str(), i);
        if (!items)
        {
            cma_free(group_name);
            json_free_groups(head);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        cma_free(group_name);
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
        skip_ws(content.c_str(), i);
        if (i < content.size() && content[i] == ',')
        {
            i++;
            continue;
        }
    }
    return (head);
}

