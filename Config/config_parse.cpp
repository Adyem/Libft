#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>
#include <cstring>
#include <cctype>

static char *trim_whitespace(char *str)
{
    if (!str)
        return str;
    while (*str && std::isspace(static_cast<unsigned char>(*str)))
        str++;
    char *end = str + std::strlen(str);
    while (end > str && std::isspace(static_cast<unsigned char>(end[-1])))
        end--;
    *end = '\0';
    return str;
}

void ft_config_free(ft_config *config)
{
    if (!config)
        return ;
    for (size_t i = 0; i < config->entry_count; ++i)
    {
        cma_free(config->entries[i].section);
        cma_free(config->entries[i].key);
        cma_free(config->entries[i].value);
    }
    cma_free(config->entries);
    cma_free(config);
    return ;
}

ft_config *ft_config_parse(const char *filename)
{
    if (!filename)
        return ft_nullptr;
    FILE *file = ft_fopen(filename, "r");
    if (!file)
        return ft_nullptr;
    ft_config *config = static_cast<ft_config*>(cma_calloc(1, sizeof(ft_config)));
    if (!config)
    {
        ft_errno = FT_EALLOC;
        ft_fclose(file);
        return ft_nullptr;
    }
    char buffer[512];
    char *current_section = ft_nullptr;
    while (std::fgets(buffer, sizeof(buffer), file))
    {
        char *line = trim_whitespace(buffer);
        if (*line == '\0' || *line == ';' || *line == '#')
            continue ;
        if (*line == '[')
        {
            char *close = std::strchr(line, ']');
            if (close)
            {
                *close = '\0';
                cma_free(current_section);
                current_section = cma_strdup(line + 1);
                if (!current_section)
                {
                    ft_errno = FT_EALLOC;
                    ft_config_free(config);
                    ft_fclose(file);
                    return ft_nullptr;
                }
            }
            continue ;
        }
        char *eq = std::strchr(line, '=');
        char *key = ft_nullptr;
        char *value = ft_nullptr;
        if (eq)
        {
            *eq = '\0';
            char *k = trim_whitespace(line);
            char *v = trim_whitespace(eq + 1);
            if (*k)
            {
                key = cma_strdup(k);
                if (!key)
                {
                    ft_errno = FT_EALLOC;
                    cma_free(value);
                    ft_config_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return ft_nullptr;
                }
            }
            if (*v)
            {
                value = cma_strdup(v);
                if (!value)
                {
                    ft_errno = FT_EALLOC;
                    cma_free(key);
                    ft_config_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return ft_nullptr;
                }
            }
        }
        else
        {
            char *k = trim_whitespace(line);
            if (*k)
            {
                key = cma_strdup(k);
                if (!key)
                {
                    ft_errno = FT_EALLOC;
                    ft_config_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return ft_nullptr;
                }
            }
        }
        ft_config_entry entry;
        entry.section = current_section ? cma_strdup(current_section) : ft_nullptr;
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
            return ft_nullptr;
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
            return ft_nullptr;
        }
        config->entries = new_entries;
        config->entries[config->entry_count] = entry;
        config->entry_count++;
    }
    if (current_section)
        cma_free(current_section);
    ft_fclose(file);
    return config;
}

