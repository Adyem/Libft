#include <cctype>
#include "../Libft/libft.hpp"
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

static char *parse_string(const char *s, size_t &i)
{
    size_t len = ft_strlen_size_t(s);
    if (i >= len || s[i] != '"')
        return (ft_nullptr);
    i++;
    size_t start = i;
    while (i < len && s[i] != '"')
        i++;
    char *result = cma_substr(s, static_cast<unsigned int>(start), i - start);
    if (i < len && s[i] == '"')
        i++;
    return (result);
}

static char *parse_number(const char *s, size_t &i)
{
    size_t len = ft_strlen_size_t(s);
    size_t start = i;
    if (i < len && (s[i] == '-' || s[i] == '+'))
        i++;
    bool has_digits = false;
    while (i < len && std::isdigit(static_cast<unsigned char>(s[i])))
    {
        i++;
        has_digits = true;
    }
    if (i < len && s[i] == '.')
    {
        i++;
        while (i < len && std::isdigit(static_cast<unsigned char>(s[i])))
            i++;
    }
    if (i < len && (s[i] == 'e' || s[i] == 'E'))
    {
        i++;
        if (i < len && (s[i] == '-' || s[i] == '+'))
            i++;
        while (i < len && std::isdigit(static_cast<unsigned char>(s[i])))
            i++;
    }
    if (!has_digits)
        return (ft_nullptr);
    return (cma_substr(s, static_cast<unsigned int>(start), i - start));
}

static char *parse_value(const char *s, size_t &i)
{
    skip_ws(s, i);
    size_t len = ft_strlen_size_t(s);
    if (i >= len)
        return (ft_nullptr);
    if (s[i] == '"')
        return (parse_string(s, i));
    if (len - i >= 4 && ft_strncmp(s + i, "true", 4) == 0)
    {
        i += 4;
        return (cma_strdup("true"));
    }
    if (len - i >= 5 && ft_strncmp(s + i, "false", 5) == 0)
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
    size_t len = ft_strlen_size_t(s);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_ws(s, i);
    if (i >= len || s[i] != '{')
        return (ft_nullptr);
    i++;
    while (i < len)
    {
        skip_ws(s, i);
        if (i < len && s[i] == '}')
        {
            i++;
            break;
        }
        char *key = parse_string(s, i);
        if (!key)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        skip_ws(s, i);
        if (i >= len || s[i] != ':')
        {
            cma_free(key);
            break;
        }
        i++;
        skip_ws(s, i);
        char *value = parse_value(s, i);
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
        if (i < len && s[i] == ',')
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
    char *content = cma_strdup("");
    if (!content)
    {
        for (int idx = 0; lines[idx]; ++idx)
            cma_free(lines[idx]);
        cma_free(lines);
        return (ft_nullptr);
    }
    for (int idx = 0; lines[idx]; ++idx)
    {
        char *tmp = cma_strjoin(content, lines[idx]);
        cma_free(content);
        cma_free(lines[idx]);
        if (!tmp)
        {
            for (int j = idx + 1; lines[j]; ++j)
                cma_free(lines[j]);
            cma_free(lines);
            return (ft_nullptr);
        }
        content = tmp;
    }
    cma_free(lines);
    size_t i = 0;
    skip_ws(content, i);
    size_t len = ft_strlen_size_t(content);
    if (i >= len || content[i] != '{')
    {
        cma_free(content);
        return (ft_nullptr);
    }
    i++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (i < len)
    {
        skip_ws(content, i);
        if (i < len && content[i] == '}')
        {
            i++;
            break;
        }
        char *group_name = parse_string(content, i);
        if (!group_name)
        {
            json_free_groups(head);
            cma_free(content);
            return (ft_nullptr);
        }
        skip_ws(content, i);
        if (i >= len || content[i] != ':')
        {
            cma_free(group_name);
            break;
        }
        i++;
        json_item *items = parse_items(content, i);
        if (!items)
        {
            cma_free(group_name);
            json_free_groups(head);
            cma_free(content);
            return (ft_nullptr);
        }
        json_group *group = json_create_json_group(group_name);
        cma_free(group_name);
        if (!group)
        {
            json_free_items(items);
            json_free_groups(head);
            cma_free(content);
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
        if (i < len && content[i] == ',')
        {
            i++;
            continue;
        }
    }
    cma_free(content);
    return (head);
}

json_group *json_read_from_string(const char *content)
{
    if (!content)
        return (ft_nullptr);
    size_t i = 0;
    skip_ws(content, i);
    size_t len = ft_strlen_size_t(content);
    if (i >= len || content[i] != '{')
        return (ft_nullptr);
    i++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (i < len)
    {
        skip_ws(content, i);
        if (i < len && content[i] == '}')
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
        skip_ws(content, i);
        if (i >= len || content[i] != ':')
        {
            cma_free(group_name);
            break;
        }
        i++;
        json_item *items = parse_items(content, i);
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
        skip_ws(content, i);
        if (i < len && content[i] == ',')
        {
            i++;
            continue;
        }
    }
    return (head);
}

