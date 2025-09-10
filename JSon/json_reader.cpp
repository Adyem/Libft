#include <cctype>
#include "../Libft/libft.hpp"
#include "json.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static void skip_whitespace(const char *json_string, size_t &index)
{
    while (json_string[index]
        && std::isspace(static_cast<unsigned char>(json_string[index])))
        index++;
    return ;
}

static char *parse_string(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length || json_string[index] != '"')
        return (ft_nullptr);
    index++;
    size_t start_index = index;
    while (index < length && json_string[index] != '"')
        index++;
    char *result = cma_substr(json_string,
                               static_cast<unsigned int>(start_index),
                               index - start_index);
    if (index < length && json_string[index] == '"')
        index++;
    return (result);
}

static char *parse_number(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    size_t start_index = index;
    if (index < length && (json_string[index] == '-' || json_string[index] == '+'))
        index++;
    bool has_digits = false;
    while (index < length
        && std::isdigit(static_cast<unsigned char>(json_string[index])))
    {
        index++;
        has_digits = true;
    }
    if (index < length && json_string[index] == '.')
    {
        index++;
        while (index < length
            && std::isdigit(static_cast<unsigned char>(json_string[index])))
            index++;
    }
    if (index < length && (json_string[index] == 'e' || json_string[index] == 'E'))
    {
        index++;
        if (index < length && (json_string[index] == '-' || json_string[index] == '+'))
            index++;
        while (index < length
            && std::isdigit(static_cast<unsigned char>(json_string[index])))
            index++;
    }
    if (!has_digits)
        return (ft_nullptr);
    return (cma_substr(json_string, static_cast<unsigned int>(start_index),
                       index - start_index));
}

static char *parse_value(const char *json_string, size_t &index)
{
    skip_whitespace(json_string, index);
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length)
        return (ft_nullptr);
    if (json_string[index] == '"')
        return (parse_string(json_string, index));
    if (length - index >= 4 && ft_strncmp(json_string + index, "true", 4) == 0)
    {
        index += 4;
        return (cma_strdup("true"));
    }
    if (length - index >= 5 && ft_strncmp(json_string + index, "false", 5) == 0)
    {
        index += 5;
        return (cma_strdup("false"));
    }
    if (std::isdigit(static_cast<unsigned char>(json_string[index]))
        || json_string[index] == '-' || json_string[index] == '+')
        return (parse_number(json_string, index));
    return (ft_nullptr);
}

static json_item *parse_items(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_whitespace(json_string, index);
    if (index >= length || json_string[index] != '{')
        return (ft_nullptr);
    index++;
    while (index < length)
    {
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == '}')
        {
            index++;
            break;
        }
        char *key = parse_string(json_string, index);
        if (!key)
        {
            json_free_items(head);
            return (ft_nullptr);
        }
        skip_whitespace(json_string, index);
        if (index >= length || json_string[index] != ':')
        {
            cma_free(key);
            break;
        }
        index++;
        skip_whitespace(json_string, index);
        char *value = parse_value(json_string, index);
        if (!value)
        {
            cma_free(key);
            json_free_items(head);
            return (ft_nullptr);
        }
        json_item *item = json_create_item(key, value);
        cma_free(key);
        if (!head)
            head = tail = item;
        else
        {
            tail->next = item;
            tail = item;
        }
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == ',')
        {
            index++;
            continue;
        }
    }
    return (head);
}

json_group *json_read_from_file(const char *filename)
{
    char **lines = ft_open_and_read_file(filename, 512);
    if (!lines)
        return (ft_nullptr);
    char *content = cma_strdup("");
    if (!content)
    {
        int line_index = 0;
        while (lines[line_index])
        {
            cma_free(lines[line_index]);
            ++line_index;
        }
        cma_free(lines);
        return (ft_nullptr);
    }
    int line_index = 0;
    while (lines[line_index])
    {
        char *tmp = cma_strjoin(content, lines[line_index]);
        cma_free(content);
        cma_free(lines[line_index]);
        if (!tmp)
        {
            int remaining_index = line_index + 1;
            while (lines[remaining_index])
            {
                cma_free(lines[remaining_index]);
                ++remaining_index;
            }
            cma_free(lines);
            return (ft_nullptr);
        }
        content = tmp;
        ++line_index;
    }
    cma_free(lines);
    size_t index = 0;
    skip_whitespace(content, index);
    size_t length = ft_strlen_size_t(content);
    if (index >= length || content[index] != '{')
    {
        cma_free(content);
        return (ft_nullptr);
    }
    index++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            break;
        }
        char *group_name = parse_string(content, index);
        if (!group_name)
        {
            json_free_groups(head);
            cma_free(content);
            return (ft_nullptr);
        }
        skip_whitespace(content, index);
        if (index >= length || content[index] != ':')
        {
            cma_free(group_name);
            break;
        }
        index++;
        json_item *items = parse_items(content, index);
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
        skip_whitespace(content, index);
        if (index < length && content[index] == ',')
        {
            index++;
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
    size_t index = 0;
    skip_whitespace(content, index);
    size_t length = ft_strlen_size_t(content);
    if (index >= length || content[index] != '{')
        return (ft_nullptr);
    index++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            break;
        }
        char *group_name = parse_string(content, index);
        if (!group_name)
        {
            json_free_groups(head);
            return (ft_nullptr);
        }
        skip_whitespace(content, index);
        if (index >= length || content[index] != ':')
        {
            cma_free(group_name);
            break;
        }
        index++;
        json_item *items = parse_items(content, index);
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
        skip_whitespace(content, index);
        if (index < length && content[index] == ',')
        {
            index++;
            continue;
        }
    }
    return (head);
}

