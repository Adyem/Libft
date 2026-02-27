#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Advanced/advanced.hpp"
#include "../File/file_utils.hpp"
#include "../JSon/json.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>
#include <cstdio>

static int config_mutex_lock(pt_mutex *mutex, bool *lock_acquired)
{
    int lock_result;

    lock_result = pt_mutex_lock_if_not_null(mutex);
    if (lock_result == FT_ERR_SUCCESS && lock_acquired)
    {
        if (mutex != ft_nullptr)
            *lock_acquired = true;
    }
    return (lock_result);
}

static int config_lock_if_enabled(config_data *config, bool *lock_acquired)
{
    if (!config || !config->mutex)
        return (FT_ERR_SUCCESS);
    return (config_mutex_lock(config->mutex, lock_acquired));
}
static int config_mutex_unlock(pt_mutex *mutex)
{
    return (pt_mutex_unlock_if_not_null(mutex));
}

static int config_mutex_create(pt_mutex **mutex_pointer)
{
    if (!mutex_pointer)
        return (FT_ERR_INVALID_ARGUMENT);
    if (*mutex_pointer != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_mutex *mutex = new (std::nothrow) pt_mutex();
    if (!mutex)
        return (FT_ERR_NO_MEMORY);
    int initialize_error = mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex;
        return (initialize_error);
    }
    *mutex_pointer = mutex;
    return (FT_ERR_SUCCESS);
}

static void config_mutex_destroy(pt_mutex **mutex_pointer)
{
    if (!mutex_pointer || *mutex_pointer == ft_nullptr)
        return ;
    pt_mutex *mutex = *mutex_pointer;
    mutex->destroy();
    delete mutex;
    *mutex_pointer = ft_nullptr;
}

static void config_unlock_guard(config_data *config, bool lock_acquired)
{
    if (!config || !config->mutex || !lock_acquired)
        return ;
    config_mutex_unlock(config->mutex);
    return ;
}

config_data *config_data_create()
{
    config_data *config;

    config = static_cast<config_data*>(adv_calloc(1, sizeof(config_data)));
    if (!config)
    {
        return (ft_nullptr);
    }
    if (config_data_prepare_thread_safety(config) != 0)
    {
        cma_free(config);
        return (ft_nullptr);
    }
    return (config);
}

int config_data_prepare_thread_safety(config_data *config)
{
    if (!config)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (config->mutex)
    {
        return (FT_ERR_SUCCESS);
    }
    int mutex_error = config_mutex_create(&config->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        return (mutex_error);
    }
    return (FT_ERR_SUCCESS);
}

void config_data_teardown_thread_safety(config_data *config)
{
    if (!config)
        return ;
    config_mutex_destroy(&config->mutex);
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

void config_data_free(config_data *config)
{
    if (!config)
    {
        return ;
    }
    bool mutex_locked = false;
    size_t entry_index;

    bool already_owned = false;
    int lock_result = config_lock_if_enabled(config, &mutex_locked);
    if (lock_result != FT_ERR_SUCCESS)
    {
        if (lock_result == FT_ERR_MUTEX_ALREADY_LOCKED && config->mutex)
        {
            already_owned = true;
        }
        else
        {
            config_data_teardown_thread_safety(config);
            cma_free(config);
            return ;
        }
    }
    entry_index = 0;
    while (entry_index < config->entry_count)
    {
        cma_free(config->entries[entry_index].section);
        cma_free(config->entries[entry_index].key);
        cma_free(config->entries[entry_index].value);
        config_entry_teardown_thread_safety(&config->entries[entry_index]);
        ++entry_index;
    }
    cma_free(config->entries);
    if (already_owned)
        config_mutex_unlock(config->mutex);
    else
        config_unlock_guard(config, mutex_locked);
    config_data_teardown_thread_safety(config);
    cma_free(config);
    return ;
}

config_data *config_parse(const char *filename)
{
    if (!filename)
    {
        return (ft_nullptr);
    }
    FILE *file = ft_fopen(filename, "r");
    if (!file)
        return (ft_nullptr);
    config_data *config = config_data_create();
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
                    current_section = adv_strdup(line_string + 1);
                    if (!current_section)
                    {
                        config_data_free(config);
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
                key = adv_strdup(key_start);
                if (!key)
                {
                    cma_free(value);
                    config_data_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
            if (*value_start)
            {
                value = adv_strdup(value_start);
                if (!value)
                {
                    cma_free(key);
                    config_data_free(config);
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
                key = adv_strdup(key_start);
                if (!key)
                {
                    config_data_free(config);
                    if (current_section)
                        cma_free(current_section);
                    ft_fclose(file);
                    return (ft_nullptr);
                }
            }
        }
        config_entry *new_entries = static_cast<config_entry*>(cma_realloc(config->entries, sizeof(config_entry) * (config->entry_count + 1)));
        if (!new_entries)
        {
            cma_free(key);
            cma_free(value);
            config_data_free(config);
            if (current_section)
                cma_free(current_section);
            ft_fclose(file);
            return (ft_nullptr);
        }
        config->entries = new_entries;
        config_entry *new_entry = &config->entries[config->entry_count];
        new_entry->mutex = ft_nullptr;
        if (current_section)
        {
            new_entry->section = adv_strdup(current_section);
            if (!new_entry->section)
            {
                cma_free(key);
                cma_free(value);
                new_entry->key = ft_nullptr;
                new_entry->value = ft_nullptr;
                config_data_free(config);
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
        if (config_entry_prepare_thread_safety(new_entry) != 0)
        {
            cma_free(new_entry->section);
            cma_free(new_entry->key);
            cma_free(new_entry->value);
            new_entry->section = ft_nullptr;
            new_entry->key = ft_nullptr;
            new_entry->value = ft_nullptr;
            config_data_free(config);
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
    return (config);
}

static config_data *config_parse_json_internal(const char *filename)
{
    if (!filename)
    {
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
    config_data *config = config_data_create();
    if (!config)
    {
        json_free_groups(groups);
        return (ft_nullptr);
    }
    if (count)
    {
        config->entries = static_cast<config_entry*>(adv_calloc(count, sizeof(config_entry)));
        if (!config->entries)
        {
            config_data_teardown_thread_safety(config);
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
            config_entry *entry = &config->entries[index];
            entry->mutex = ft_nullptr;
            if (group_pointer->name)
            {
                entry->section = adv_strdup(group_pointer->name);
                if (!entry->section)
                {
                    config->entry_count = index;
                    config_data_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (item_pointer->key)
            {
                entry->key = adv_strdup(item_pointer->key);
                if (!entry->key)
                {
                    config->entry_count = index + 1;
                    config_data_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (item_pointer->value)
            {
                entry->value = adv_strdup(item_pointer->value);
                if (!entry->value)
                {
                    config->entry_count = index + 1;
                    config_data_free(config);
                    json_free_groups(groups);
                    return (ft_nullptr);
                }
            }
            if (config_entry_prepare_thread_safety(entry) != 0)
            {
                config->entry_count = index + 1;
                config_data_free(config);
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
    return (config);
}

config_data *config_load_env()
{
    extern char **environ;
    config_data *config = config_data_create();
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
        return (config);
    }
    config->entries = static_cast<config_entry*>(adv_calloc(count, sizeof(config_entry)));
    if (!config->entries)
    {
        config_data_teardown_thread_safety(config);
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
        config_entry *entry = &config->entries[index];
        entry->mutex = ft_nullptr;
        if (equals_sign)
        {
            size_t key_length = static_cast<size_t>(equals_sign - pair);
            entry->key = static_cast<char*>(adv_calloc(key_length + 1, sizeof(char)));
            if (!entry->key)
            {
                config->entry_count = index;
                config_data_free(config);
                return (ft_nullptr);
            }
            ft_memcpy(entry->key, pair, key_length);
            if (equals_sign[1])
            {
                entry->value = adv_strdup(equals_sign + 1);
                if (!entry->value)
                {
                    config->entry_count = index + 1;
                    config_data_free(config);
                    return (ft_nullptr);
                }
            }
        }
        else if (pair)
        {
            entry->key = adv_strdup(pair);
            if (!entry->key)
            {
                config->entry_count = index;
                config_data_free(config);
                return (ft_nullptr);
            }
        }
        entry->section = ft_nullptr;
        if (config_entry_prepare_thread_safety(entry) != 0)
        {
            config->entry_count = index + 1;
            config_data_free(config);
            return (ft_nullptr);
        }
        ++index;
    }
    config->entry_count = count;
    return (config);
}

config_data *config_load_file(const char *filename)
{
    config_data *config;

    if (!filename)
    {
        return (ft_nullptr);
    }
    const char *dot = ft_strrchr(filename, '.');
    if (dot && ft_strcmp(dot, ".json") == 0)
    {
        config = config_parse_json_internal(filename);
        if (config)
            return (config);
    }
    config = config_parse(filename);
    if (config)
        return (config);
    return (ft_nullptr);
}
