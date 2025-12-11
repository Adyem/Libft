#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../JSon/json.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/mutex.hpp"
#include <cstdio>
#include <new>

static int cnfg_config_lock_if_enabled(cnfg_config *config, ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!config || !config->thread_safe_enabled || !config->mutex)
        return (FT_ERR_SUCCESSS);
    mutex_guard = ft_unique_lock<pt_mutex>(*config->mutex);
    if (mutex_guard.get_error() != FT_ERR_SUCCESSS)
        return (mutex_guard.get_error());
    return (FT_ERR_SUCCESSS);
}

static void cnfg_config_unlock_guard(ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!mutex_guard.owns_lock())
        return ;
    mutex_guard.unlock();
    return ;
}

cnfg_config *cnfg_config_create()
{
    cnfg_config *config;

    config = static_cast<cnfg_config*>(cma_calloc(1, sizeof(cnfg_config)));
    if (!config)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (cnfg_config_prepare_thread_safety(config) != 0)
    {
        cma_free(config);
        return (ft_nullptr);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (config);
}

int cnfg_config_prepare_thread_safety(cnfg_config *config)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!config)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (config->thread_safe_enabled && config->mutex)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    config->mutex = mutex_pointer;
    config->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void cnfg_config_teardown_thread_safety(cnfg_config *config)
{
    if (!config)
        return ;
    if (config->mutex)
    {
        config->mutex->~pt_mutex();
        cma_free(config->mutex);
        config->mutex = ft_nullptr;
    }
    config->thread_safe_enabled = false;
    return ;
}

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
    ft_unique_lock<pt_mutex> mutex_guard;
    int entry_errno;
    size_t entry_index;

    entry_errno = ft_errno;
    bool already_owned = false;
    int lock_result = cnfg_config_lock_if_enabled(config, mutex_guard);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        if (lock_result == FT_ERR_MUTEX_ALREADY_LOCKED && config->mutex)
        {
            already_owned = true;
            ft_errno = entry_errno;
        }
        else
        {
            cnfg_config_teardown_thread_safety(config);
            cma_free(config);
            ft_errno = entry_errno;
            return ;
        }
    }
    entry_index = 0;
    while (entry_index < config->entry_count)
    {
        cma_free(config->entries[entry_index].section);
        cma_free(config->entries[entry_index].key);
        cma_free(config->entries[entry_index].value);
        cnfg_entry_teardown_thread_safety(&config->entries[entry_index]);
        ++entry_index;
    }
    cma_free(config->entries);
    ft_errno = entry_errno;
    if (already_owned)
    {
        config->mutex->unlock(THREAD_ID);
        if (config->mutex->get_error() != FT_ERR_SUCCESSS)
            ft_errno = config->mutex->get_error();
    }
    else
        cnfg_config_unlock_guard(mutex_guard);
    cnfg_config_teardown_thread_safety(config);
    cma_free(config);
    return ;
}

cnfg_config *cnfg_parse(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    FILE *file = ft_fopen(filename, "r");
    if (!file)
        return (ft_nullptr);
    cnfg_config *config = cnfg_config_create();
    if (!config)
    {
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
                current_section = ft_nullptr;
                if (*(line_string + 1))
                {
                    current_section = cma_strdup(line_string + 1);
                    if (!current_section)
                    {
                        ft_errno = FT_ERR_NO_MEMORY;
                        cnfg_free(config);
                        ft_fclose(file);
                        return (ft_nullptr);
                    }
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
                    ft_errno = FT_ERR_NO_MEMORY;
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
                    ft_errno = FT_ERR_NO_MEMORY;
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
                    ft_errno = FT_ERR_NO_MEMORY;
                    cnfg_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
        }
        cnfg_entry *new_entries = static_cast<cnfg_entry*>(cma_realloc(config->entries, sizeof(cnfg_entry) * (config->entry_count + 1)));
        if (!new_entries)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            cma_free(key);
            cma_free(value);
            cnfg_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        config->entries = new_entries;
        cnfg_entry *new_entry = &config->entries[config->entry_count];
        new_entry->mutex = ft_nullptr;
        new_entry->thread_safe_enabled = false;
        if (current_section)
        {
            new_entry->section = cma_strdup(current_section);
            if (!new_entry->section)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                cma_free(key);
                cma_free(value);
                new_entry->key = ft_nullptr;
                new_entry->value = ft_nullptr;
                cnfg_free(config);
                if (current_section)
                    cma_free(current_section);
                ft_fclose(file);
                return (ft_nullptr);
            }
        }
        else
            new_entry->section = ft_nullptr;
        new_entry->key = key;
        new_entry->value = value;
        if (cnfg_entry_prepare_thread_safety(new_entry) != 0)
        {
            cma_free(new_entry->section);
            cma_free(new_entry->key);
            cma_free(new_entry->value);
            new_entry->section = ft_nullptr;
            new_entry->key = ft_nullptr;
            new_entry->value = ft_nullptr;
            cnfg_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        config->entry_count++;
    }
    if (current_section)
        cma_free(current_section);
    ft_fclose(file);
    ft_errno = FT_ERR_SUCCESSS;
    return (config);
}

static cnfg_config *cnfg_parse_json(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    FILE *file;
    json_group *groups;

    file = ft_fopen(filename, "r");
    if (!file)
        return (ft_nullptr);
    groups = json_read_from_file_stream(file, 512);
    ft_fclose(file);
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
    cnfg_config *config = cnfg_config_create();
    if (!config)
    {
        json_free_groups(groups);
        return (ft_nullptr);
    }
    if (count)
    {
        config->entries = static_cast<cnfg_entry*>(cma_calloc(count, sizeof(cnfg_entry)));
        if (!config->entries)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            cnfg_config_teardown_thread_safety(config);
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
            entry->mutex = ft_nullptr;
            entry->thread_safe_enabled = false;
            if (group_pointer->name)
            {
                entry->section = cma_strdup(group_pointer->name);
                if (!entry->section)
                {
                    ft_errno = FT_ERR_NO_MEMORY;
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
                    ft_errno = FT_ERR_NO_MEMORY;
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
                    ft_errno = FT_ERR_NO_MEMORY;
                    config->entry_count = index + 1;
                    cnfg_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (cnfg_entry_prepare_thread_safety(entry) != 0)
            {
                config->entry_count = index + 1;
                cnfg_free(config);
                json_free_groups(groups);
                return (ft_nullptr);
            }
            ++index;
            item_pointer = item_pointer->next;
        }
        group_pointer = group_pointer->next;
    }
    config->entry_count = count;
    json_free_groups(groups);
    ft_errno = FT_ERR_SUCCESSS;
    return (config);
}

cnfg_config *config_load_env()
{
    extern char **environ;
    cnfg_config *config = cnfg_config_create();
    if (!config)
        return (ft_nullptr);
    size_t count = 0;
    if (environ)
    {
        while (environ[count])
            ++count;
    }
    if (!count)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (config);
    }
    config->entries = static_cast<cnfg_entry*>(cma_calloc(count, sizeof(cnfg_entry)));
    if (!config->entries)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        cnfg_config_teardown_thread_safety(config);
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
        entry->mutex = ft_nullptr;
        entry->thread_safe_enabled = false;
        if (equals_sign)
        {
            size_t key_length = static_cast<size_t>(equals_sign - pair);
            entry->key = static_cast<char*>(cma_calloc(key_length + 1, sizeof(char)));
            if (!entry->key)
            {
                ft_errno = FT_ERR_NO_MEMORY;
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
                    ft_errno = FT_ERR_NO_MEMORY;
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
                ft_errno = FT_ERR_NO_MEMORY;
                config->entry_count = index;
                cnfg_free(config);
                return (ft_nullptr);
            }
        }
        entry->section = ft_nullptr;
        if (cnfg_entry_prepare_thread_safety(entry) != 0)
        {
            config->entry_count = index + 1;
            cnfg_free(config);
            return (ft_nullptr);
        }
        ++index;
    }
    config->entry_count = count;
    ft_errno = FT_ERR_SUCCESSS;
    return (config);
}

cnfg_config *config_load_file(const char *filename)
{
    if (!filename)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    const char *dot = ft_strrchr(filename, '.');
    if (dot && ft_strcmp(dot, ".json") == 0)
    {
        cnfg_config *config = cnfg_parse_json(filename);
        if (config)
            ft_errno = FT_ERR_SUCCESSS;
        return (config);
    }
    cnfg_config *config = cnfg_parse(filename);
    if (config)
        ft_errno = FT_ERR_SUCCESSS;
    return (config);
}

