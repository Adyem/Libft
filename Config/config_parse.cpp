#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>
#include <cstring>
#include <cctype>

static char *trim_whitespace(char *string)
{
    if (!string)
        return (string);
    while (*string && std::isspace(static_cast<unsigned char>(*string)))
        string++;
    char *end_pointer = string + std::strlen(string);
    while (end_pointer > string && std::isspace(static_cast<unsigned char>(end_pointer[-1])))
        end_pointer--;
    *end_pointer = '\0';
    return (string);
}

void ft_config_free(ft_config *config)
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

ft_config *ft_config_parse(const char *filename)
{
    if (!filename)
        return (ft_nullptr);
    FILE *file = ft_fopen(filename, "r");
    if (!file)
        return (ft_nullptr);
    ft_config *config = static_cast<ft_config*>(cma_calloc(1, sizeof(ft_config)));
    if (!config)
    {
        ft_errno = FT_EALLOC;
        ft_fclose(file);
        return (ft_nullptr);
    }
    char buffer[512];
    char *current_section = ft_nullptr;
    while (std::fgets(buffer, sizeof(buffer), file))
    {
        char *line_string = trim_whitespace(buffer);
        if (*line_string == '\0' || *line_string == ';' || *line_string == '#')
            continue ;
        if (*line_string == '[')
        {
            char *closing_bracket = std::strchr(line_string, ']');
            if (closing_bracket)
            {
                *closing_bracket = '\0';
                cma_free(current_section);
                current_section = cma_strdup(line_string + 1);
                if (!current_section)
                {
                    ft_errno = FT_EALLOC;
                    ft_config_free(config);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
            continue ;
        }
        char *equals_sign = std::strchr(line_string, '=');
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
                    ft_config_free(config);
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
                    ft_config_free(config);
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
                    ft_config_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
        }
        ft_config_entry entry;
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
            cma_free(value);
            ft_config_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        ft_config_entry *new_entries = static_cast<ft_config_entry*>(cma_realloc(config->entries, sizeof(ft_config_entry) * (config->entry_count + 1)));
        if (!new_entries)
        {
            ft_errno = FT_EALLOC;
            cma_free(entry.section);
            cma_free(entry.key);
            cma_free(entry.value);
            ft_config_free(config);
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
    return (config);
}

