#include "config.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../File/file_utils.hpp"
#include "../JSon/json.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t config_lock_if_enabled(const config_data *config,
    ft_bool *lock_acquired)
{
    int32_t lock_result;

    if (!config)
        return (FT_ERR_SUCCESS);
    lock_result = pt_mutex_lock_if_not_null(config->mutex);
    if (lock_result == FT_ERR_SUCCESS && lock_acquired)
    {
        if (config->mutex != ft_nullptr)
            *lock_acquired = FT_TRUE;
    }
    return (lock_result);
}

static void config_unlock_guard(const config_data *config, ft_bool lock_acquired)
{
    if (!config || lock_acquired == FT_FALSE)
        return ;
    (void)pt_mutex_unlock_if_not_null(config->mutex);
    return ;
}

static int32_t config_handle_write_failure(FILE *file)
{
    if (file)
        ft_fclose(file);
    return (FT_ERR_IO);
}

static int32_t config_write_ini(const config_data *config, const char *filename)
{
    FILE *file;
    const char *last_section;
    ft_size_t entry_index;

    file = ft_fopen(filename, "w");
    if (!file)
        return (FT_ERR_FILE_OPEN_FAILED);
    last_section = ft_nullptr;
    entry_index = 0;
    while (config && entry_index < config->entry_count)
    {
        const config_entry *entry = &config->entries[entry_index];
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
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

static json_group *config_find_or_create_group(json_group **groups_head, const char *section_name)
{
    const char *name;
    json_group *current;

    if (!groups_head)
    {
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

static int32_t config_write_json(const config_data *config, const char *filename)
{
    json_group *groups;
    ft_size_t entry_index;

    groups = ft_nullptr;
    entry_index = 0;
    while (config && entry_index < config->entry_count)
    {
        const config_entry *entry = &config->entries[entry_index];
        if (!entry->key || !entry->value)
        {
            json_free_groups(groups);
            return (FT_ERR_INVALID_ARGUMENT);
        }
        json_group *group = config_find_or_create_group(&groups, entry->section);
        if (!group)
        {
            json_free_groups(groups);
            return (FT_ERR_NO_MEMORY);
        }
        json_item *item = json_create_item(entry->key, entry->value);
        if (!item)
        {
            json_free_groups(groups);
            return (FT_ERR_NO_MEMORY);
        }
        json_add_item_to_group(group, item);
        ++entry_index;
    }
    if (json_write_to_file(filename, groups) != FT_ERR_SUCCESS)
    {
        json_free_groups(groups);
        return (FT_ERR_IO);
    }
    json_free_groups(groups);
    return (FT_ERR_SUCCESS);
}

int32_t config_write_file(const config_data *config, const char *filename)
{
    const char *extension;
    ft_bool mutex_locked;
    int32_t lock_error;

    mutex_locked = FT_FALSE;
    if (!config || !filename)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_error = config_lock_if_enabled(config, &mutex_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    if (config->entry_count && !config->entries)
    {
        config_unlock_guard(config, mutex_locked);
        return (FT_ERR_INVALID_STATE);
    }
    extension = ft_strrchr(filename, '.');
    if (extension && ft_strcmp(extension, ".json") == 0)
    {
        int32_t write_result;

        write_result = config_write_json(config, filename);
        config_unlock_guard(config, mutex_locked);
        return (write_result);
    }
    int32_t write_result;

    write_result = config_write_ini(config, filename);
    config_unlock_guard(config, mutex_locked);
    return (write_result);
}
