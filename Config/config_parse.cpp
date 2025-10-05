#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../JSon/json.hpp"
#include <cstdio>

static char *trim_whitespace(char *string)
{
    if (!string)
        return (string);
    while (*string && ft_isspace(static_cast<unsigned char>(*string)))
        string++;
    char *end_pointer = string + ft_strlen(string);
    while (end_pointer > string && ft_isspace(static_cast<unsigned char>(end_pointer[-1])))
        end_pointer--;
    *end_pointer = '\0';
    return (string);
}

void cnfg_free(cnfg_config *config)
{
    if (!config)
        return ;
    size_t entry_index = 0;
    while (entry_index < config->entry_count)
    {
        cma_free(config->entries[entry_index].section);
        cma_free(config->entries[entry_index].key);
        cma_free(config->entries[entry_index].value);
        ++entry_index;
    }
    cma_free(config->entries);
    cma_free(config);
    return ;
}

cnfg_config *cnfg_parse(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    FILE *file = ft_fopen(filename, "r");
    if (!file)
        return (ft_nullptr);
    cnfg_config *config = static_cast<cnfg_config*>(cma_calloc(1, sizeof(cnfg_config)));
    if (!config)
    {
        ft_errno = FT_EALLOC;
        ft_fclose(file);
        return (ft_nullptr);
    }
    char buffer[512];
    char *current_section = ft_nullptr;
    while (ft_fgets(buffer, sizeof(buffer), file))
    {
        char *line_string = trim_whitespace(buffer);
        if (*line_string == '\0' || *line_string == ';' || *line_string == '#')
            continue ;
        if (*line_string == '[')
        {
            char *closing_bracket = ft_strchr(line_string, ']');
            if (closing_bracket)
            {
                *closing_bracket = '\0';
                cma_free(current_section);
                current_section = cma_strdup(line_string + 1);
                if (!current_section)
                {
                    ft_errno = FT_EALLOC;
                    cnfg_free(config);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
            continue ;
        }
        char *equals_sign = ft_strchr(line_string, '=');
        char *key = ft_nullptr;
        char *value = ft_nullptr;
        if (equals_sign)
        {
            *equals_sign = '\0';
            char *key_start = trim_whitespace(line_string);
            char *value_start = trim_whitespace(equals_sign + 1);
            if (*key_start)
            {
                key = cma_strdup(key_start);
                if (!key)
                {
                    ft_errno = FT_EALLOC;
                    cma_free(value);
                    cnfg_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
            if (*value_start)
            {
                value = cma_strdup(value_start);
                if (!value)
                {
                    ft_errno = FT_EALLOC;
                    cma_free(key);
                    cnfg_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
        }
        else
        {
            char *key_start = trim_whitespace(line_string);
            if (*key_start)
            {
                key = cma_strdup(key_start);
                if (!key)
                {
                    ft_errno = FT_EALLOC;
                    cnfg_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
        }
        cnfg_entry entry;
        if (current_section)
            entry.section = cma_strdup(current_section);
        else
            entry.section = ft_nullptr;
        entry.key = key;
        entry.value = value;
        if (current_section && !entry.section)
        {
            ft_errno = FT_EALLOC;
            cma_free(key);
            cma_free(entry.value);
            cnfg_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        cnfg_entry *new_entries = static_cast<cnfg_entry*>(cma_realloc(config->entries, sizeof(cnfg_entry) * (config->entry_count + 1)));
        if (!new_entries)
        {
            ft_errno = FT_EALLOC;
            cma_free(entry.section);
            cma_free(entry.key);
            cma_free(entry.value);
            cnfg_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        config->entries = new_entries;
        config->entries[config->entry_count] = entry;
        config->entry_count++;
    }
    if (current_section)
        cma_free(current_section);
    ft_fclose(file);
    ft_errno = ER_SUCCESS;
    return (config);
}

static cnfg_config *cnfg_parse_json(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    json_group *groups = json_read_from_file(filename);
    if (!groups)
        return (ft_nullptr);
    size_t count = 0;
    json_group *group_pointer = groups;
    while (group_pointer)
    {
        json_item *item_pointer = group_pointer->items;
        while (item_pointer)
        {
            ++count;
            item_pointer = item_pointer->next;
        }
        group_pointer = group_pointer->next;
    }
    cnfg_config *config = static_cast<cnfg_config*>(cma_calloc(1, sizeof(cnfg_config)));
    if (!config)
    {
        ft_errno = FT_EALLOC;
        json_free_groups(groups);
        return (ft_nullptr);
    }
    if (count)
    {
        config->entries = static_cast<cnfg_entry*>(cma_calloc(count, sizeof(cnfg_entry)));
        if (!config->entries)
        {
            ft_errno = FT_EALLOC;
            cma_free(config);
            json_free_groups(groups);
            return (ft_nullptr);
        }
    }
    size_t index = 0;
    group_pointer = groups;
    while (group_pointer)
    {
        json_item *item_pointer = group_pointer->items;
        while (item_pointer)
        {
            cnfg_entry *entry = &config->entries[index];
            if (group_pointer->name)
            {
                entry->section = cma_strdup(group_pointer->name);
                if (!entry->section)
                {
                    ft_errno = FT_EALLOC;
                    config->entry_count = index;
                    cnfg_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (item_pointer->key)
            {
                entry->key = cma_strdup(item_pointer->key);
                if (!entry->key)
                {
                    ft_errno = FT_EALLOC;
                    config->entry_count = index + 1;
                    cnfg_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (item_pointer->value)
            {
                entry->value = cma_strdup(item_pointer->value);
                if (!entry->value)
                {
                    ft_errno = FT_EALLOC;
                    config->entry_count = index + 1;
                    cnfg_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            ++index;
            item_pointer = item_pointer->next;
        }
        group_pointer = group_pointer->next;
    }
    config->entry_count = count;
    json_free_groups(groups);
    ft_errno = ER_SUCCESS;
    return (config);
}

cnfg_config *config_load_env()
{
    extern char **environ;
    cnfg_config *config = static_cast<cnfg_config*>(cma_calloc(1, sizeof(cnfg_config)));
    if (!config)
    {
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    size_t count = 0;
    if (environ)
    {
        while (environ[count])
            ++count;
    }
    if (!count)
    {
        ft_errno = ER_SUCCESS;
        return (config);
    }
    config->entries = static_cast<cnfg_entry*>(cma_calloc(count, sizeof(cnfg_entry)));
    if (!config->entries)
    {
        ft_errno = FT_EALLOC;
        cma_free(config);
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < count)
    {
        char *pair = environ[index];
        char *equals_sign = ft_nullptr;
        if (pair)
            equals_sign = ft_strchr(pair, '=');
        cnfg_entry *entry = &config->entries[index];
        if (equals_sign)
        {
            size_t key_length = static_cast<size_t>(equals_sign - pair);
            entry->key = static_cast<char*>(cma_calloc(key_length + 1, sizeof(char)));
            if (!entry->key)
            {
                ft_errno = FT_EALLOC;
                config->entry_count = index;
                cnfg_free(config);
                return (ft_nullptr);
            }
            ft_memcpy(entry->key, pair, key_length);
            if (equals_sign[1])
            {
                entry->value = cma_strdup(equals_sign + 1);
                if (!entry->value)
                {
                    ft_errno = FT_EALLOC;
                    config->entry_count = index + 1;
                    cnfg_free(config);
                    return (ft_nullptr);
                }
            }
        }
        else if (pair)
        {
            entry->key = cma_strdup(pair);
            if (!entry->key)
            {
                ft_errno = FT_EALLOC;
                config->entry_count = index;
                cnfg_free(config);
                return (ft_nullptr);
            }
        }
        entry->section = ft_nullptr;
        ++index;
    }
    config->entry_count = count;
    ft_errno = ER_SUCCESS;
    return (config);
}

cnfg_config *config_load_file(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    const char *dot = ft_strrchr(filename, '.');
    if (dot && ft_strcmp(dot, ".json") == 0)
    {
        cnfg_config *config = cnfg_parse_json(filename);
        if (config)
            ft_errno = ER_SUCCESS;
        return (config);
    }
    cnfg_config *config = cnfg_parse(filename);
    if (config)
        ft_errno = ER_SUCCESS;
    return (config);
}


