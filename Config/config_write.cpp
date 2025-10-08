#include "config.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../JSon/json.hpp"
#include <cstddef>

static int config_handle_write_failure(FILE *file)
{
    int error_code;

    error_code = ft_errno;
    if (file)
        ft_fclose(file);
    if (error_code == ER_SUCCESS)
        error_code = FT_ERR_IO;
    ft_errno = error_code;
    return (-1);
}

static int config_write_ini(const cnfg_config *config, const char *filename)
{
    FILE *file;
    const char *last_section;
    size_t entry_index;

    file = ft_fopen(filename, "w");
    if (!file)
        return (-1);
    last_section = ft_nullptr;
    entry_index = 0;
    while (config && entry_index < config->entry_count)
    {
        const cnfg_entry *entry = &config->entries[entry_index];
        if (entry->section)
        {
            if (!last_section || ft_strcmp(entry->section, last_section) != 0)
            {
                if (ft_fprintf(file, "[%s]\n", entry->section) < 0)
                    return (config_handle_write_failure(file));
            }
            last_section = entry->section;
        }
        else
        {
            if (last_section)
            {
                if (ft_fprintf(file, "[]\n") < 0)
                    return (config_handle_write_failure(file));
            }
            last_section = ft_nullptr;
        }
        if (entry->key && entry->value)
        {
            if (ft_fprintf(file, "%s=%s\n", entry->key, entry->value) < 0)
                return (config_handle_write_failure(file));
        }
        else if (entry->key)
        {
            if (ft_fprintf(file, "%s=\n", entry->key) < 0)
                return (config_handle_write_failure(file));
        }
        else if (entry->value)
        {
            if (ft_fprintf(file, "=%s\n", entry->value) < 0)
                return (config_handle_write_failure(file));
        }
        ++entry_index;
    }
    if (ft_fclose(file) == EOF)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

static json_group *config_find_or_create_group(json_group **groups_head, const char *section_name)
{
    const char *name;
    json_group *current;

    if (!groups_head)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    name = section_name;
    if (!name)
        name = "";
    current = *groups_head;
    while (current)
    {
        if (current->name && ft_strcmp(current->name, name) == 0)
            return (current);
        current = current->next;
    }
    json_group *new_group = json_create_json_group(name);
    if (!new_group)
        return (ft_nullptr);
    if (!(*groups_head))
        *groups_head = new_group;
    else
    {
        current = *groups_head;
        while (current->next)
            current = current->next;
        current->next = new_group;
    }
    return (new_group);
}

static int config_write_json(const cnfg_config *config, const char *filename)
{
    json_group *groups;
    size_t entry_index;

    groups = ft_nullptr;
    entry_index = 0;
    while (config && entry_index < config->entry_count)
    {
        const cnfg_entry *entry = &config->entries[entry_index];
        if (!entry->key || !entry->value)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            json_free_groups(groups);
            return (-1);
        }
        json_group *group = config_find_or_create_group(&groups, entry->section);
        if (!group)
        {
            json_free_groups(groups);
            return (-1);
        }
        json_item *item = json_create_item(entry->key, entry->value);
        if (!item)
        {
            json_free_groups(groups);
            return (-1);
        }
        json_add_item_to_group(group, item);
        ++entry_index;
    }
    if (json_write_to_file(filename, groups) != 0)
    {
        json_free_groups(groups);
        return (-1);
    }
    json_free_groups(groups);
    return (0);
}

int config_write_file(const cnfg_config *config, const char *filename)
{
    const char *extension;

    if (!config || !filename)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (config->entry_count && !config->entries)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    extension = ft_strrchr(filename, '.');
    if (extension && ft_strcmp(extension, ".json") == 0)
        return (config_write_json(config, filename));
    return (config_write_ini(config, filename));
}

