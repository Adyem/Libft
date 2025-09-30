#include "../Libft/libft.hpp"
#include "json.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void skip_whitespace(const char *json_string, size_t &index)
{
    while (json_string[index]
        && ft_isspace(static_cast<unsigned char>(json_string[index])))
        index++;
    return ;
}

static char *parse_string(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length || json_string[index] != '"')
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index++;
    size_t start_index = index;
    while (index < length && json_string[index] != '"')
        index++;
    if (index >= length || json_string[index] != '"')
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    char *result = cma_substr(json_string,
                               static_cast<unsigned int>(start_index),
                               index - start_index);
    if (!result)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    index++;
    ft_errno = ER_SUCCESS;
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
        && ft_isdigit(static_cast<unsigned char>(json_string[index])))
    {
        index++;
        has_digits = true;
    }
    if (index < length && json_string[index] == '.')
    {
        index++;
        while (index < length
            && ft_isdigit(static_cast<unsigned char>(json_string[index])))
            index++;
    }
    if (index < length && (json_string[index] == 'e' || json_string[index] == 'E'))
    {
        index++;
        if (index < length && (json_string[index] == '-' || json_string[index] == '+'))
            index++;
        while (index < length
            && ft_isdigit(static_cast<unsigned char>(json_string[index])))
            index++;
    }
    if (!has_digits)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    char *number = cma_substr(json_string,
                               static_cast<unsigned int>(start_index),
                               index - start_index);
    if (!number)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (number);
}

static char *parse_value(const char *json_string, size_t &index)
{
    skip_whitespace(json_string, index);
    size_t length = ft_strlen_size_t(json_string);
    if (index >= length)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    if (json_string[index] == '"')
        return (parse_string(json_string, index));
    if (length - index >= 4 && ft_strncmp(json_string + index, "true", 4) == 0)
    {
        index += 4;
        char *value = cma_strdup("true");
        if (!value)
        {
            ft_errno = JSON_MALLOC_FAIL;
            return (ft_nullptr);
        }
        ft_errno = ER_SUCCESS;
        return (value);
    }
    if (length - index >= 5 && ft_strncmp(json_string + index, "false", 5) == 0)
    {
        index += 5;
        char *value = cma_strdup("false");
        if (!value)
        {
            ft_errno = JSON_MALLOC_FAIL;
            return (ft_nullptr);
        }
        ft_errno = ER_SUCCESS;
        return (value);
    }
    if (ft_isdigit(static_cast<unsigned char>(json_string[index]))
        || json_string[index] == '-' || json_string[index] == '+')
        return (parse_number(json_string, index));
    ft_errno = FT_EINVAL;
    return (ft_nullptr);
}

static json_item *parse_items(const char *json_string, size_t &index)
{
    size_t length = ft_strlen_size_t(json_string);
    json_item *head = ft_nullptr;
    json_item *tail = ft_nullptr;
    skip_whitespace(json_string, index);
    if (index >= length || json_string[index] != '{')
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index++;
    bool object_closed = false;
    while (index < length)
    {
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == '}')
        {
            index++;
            object_closed = true;
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
            json_free_items(head);
            ft_errno = FT_EINVAL;
            return (ft_nullptr);
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
        cma_free(value);
        cma_free(key);
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
        skip_whitespace(json_string, index);
        if (index < length && json_string[index] == ',')
        {
            index++;
            continue;
        }
    }
    if (!object_closed)
    {
        json_free_items(head);
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (head);
}

json_group *json_read_from_file(const char *filename)
{
    char **lines = ft_open_and_read_file(filename, 512);
    if (!lines)
    {
        ft_errno = FT_EIO;
        return (ft_nullptr);
    }
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
        ft_errno = JSON_MALLOC_FAIL;
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
            ft_errno = JSON_MALLOC_FAIL;
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
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    bool object_closed = false;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            object_closed = true;
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
            json_free_groups(head);
            cma_free(content);
            ft_errno = FT_EINVAL;
            return (ft_nullptr);
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
    if (!object_closed)
    {
        json_free_groups(head);
        cma_free(content);
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    cma_free(content);
    ft_errno = ER_SUCCESS;
    return (head);
}

json_group *json_read_from_string(const char *content)
{
    if (!content)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    size_t index = 0;
    skip_whitespace(content, index);
    size_t length = ft_strlen_size_t(content);
    if (index >= length || content[index] != '{')
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index++;
    json_group *head = ft_nullptr;
    json_group *tail = ft_nullptr;
    bool object_closed = false;
    while (index < length)
    {
        skip_whitespace(content, index);
        if (index < length && content[index] == '}')
        {
            index++;
            object_closed = true;
            break;
        }
        char *group_name = parse_string(content, index);
        if (!group_name)
        {
            int error_code;

            error_code = ft_errno;
            json_free_groups(head);
            if (error_code == ER_SUCCESS)
                ft_errno = FT_EINVAL;
            else
                ft_errno = error_code;
            return (ft_nullptr);
        }
        skip_whitespace(content, index);
        if (index >= length || content[index] != ':')
        {
            cma_free(group_name);
            json_free_groups(head);
            ft_errno = FT_EINVAL;
            return (ft_nullptr);
        }
        index++;
        json_item *items = parse_items(content, index);
        if (!items)
        {
            int error_code;

            error_code = ft_errno;
            cma_free(group_name);
            json_free_groups(head);
            if (error_code == ER_SUCCESS)
                ft_errno = FT_EINVAL;
            else
                ft_errno = error_code;
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
    if (!object_closed)
    {
        int error_code;

        error_code = ft_errno;
        json_free_groups(head);
        if (error_code == ER_SUCCESS)
            ft_errno = FT_EINVAL;
        else
            ft_errno = error_code;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (head);
}

