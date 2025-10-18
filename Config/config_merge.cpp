#include "config.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstddef>
#include "../PThread/unique_lock.hpp"
#include "../PThread/mutex.hpp"

static int cnfg_config_lock_if_enabled(cnfg_config *config, ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!config || !config->thread_safe_enabled || !config->mutex)
        return (ER_SUCCESS);
    mutex_guard = ft_unique_lock<pt_mutex>(*config->mutex);
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (mutex_guard.get_error());
    return (ER_SUCCESS);
}

static void cnfg_config_unlock_guard(ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!mutex_guard.owns_lock())
        return ;
    mutex_guard.unlock();
    return ;
}

static void config_free_entry_contents(cnfg_entry *entry)
{
    if (!entry)
        return ;
    if (entry->section)
        cma_free(entry->section);
    if (entry->key)
        cma_free(entry->key);
    if (entry->value)
        cma_free(entry->value);
    entry->section = ft_nullptr;
    entry->key = ft_nullptr;
    entry->value = ft_nullptr;
    return ;
}

static int config_duplicate_entry(const cnfg_entry *source, cnfg_entry *destination)
{
    if (!source || !destination)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    destination->section = ft_nullptr;
    destination->key = ft_nullptr;
    destination->value = ft_nullptr;
    if (source->section)
    {
        destination->section = cma_strdup(source->section);
        if (!destination->section)
        {
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    if (source->key)
    {
        destination->key = cma_strdup(source->key);
        if (!destination->key)
        {
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    if (source->value)
    {
        destination->value = cma_strdup(source->value);
        if (!destination->value)
        {
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    return (0);
}

static int config_strings_equal(const char *left, const char *right)
{
    if (!left && !right)
        return (1);
    if (!left || !right)
        return (0);
    if (ft_strcmp(left, right) == 0)
        return (1);
    return (0);
}

static cnfg_entry *config_find_matching_entry(cnfg_config *config, const cnfg_entry *entry)
{
    size_t index;

    if (!config || !entry)
        return (ft_nullptr);
    index = 0;
    while (index < config->entry_count)
    {
        cnfg_entry *candidate = &config->entries[index];
        if (config_strings_equal(candidate->section, entry->section)
            && config_strings_equal(candidate->key, entry->key))
            return (candidate);
        ++index;
    }
    return (ft_nullptr);
}

static int config_append_entry(cnfg_config *destination, const cnfg_entry *source)
{
    cnfg_entry copy;
    cnfg_entry *new_entries;

    if (!destination || !source)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (config_duplicate_entry(source, &copy) != 0)
        return (-1);
    new_entries = static_cast<cnfg_entry*>(cma_realloc(destination->entries,
            sizeof(cnfg_entry) * (destination->entry_count + 1)));
    if (!new_entries)
    {
        config_free_entry_contents(&copy);
        return (-1);
    }
    destination->entries = new_entries;
    destination->entries[destination->entry_count] = copy;
    destination->entry_count++;
    return (0);
}

static int config_copy_entries(cnfg_config *destination, const cnfg_config *source)
{
    size_t index;

    if (!source)
        return (0);
    if (source->entry_count && !source->entries)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    index = 0;
    while (index < source->entry_count)
    {
        if (config_append_entry(destination, &source->entries[index]) != 0)
            return (-1);
        ++index;
    }
    return (0);
}

cnfg_config *config_merge(const cnfg_config *base_config, const cnfg_config *override_config)
{
    cnfg_config *result;
    size_t index;
    ft_unique_lock<pt_mutex> base_guard;
    ft_unique_lock<pt_mutex> override_guard;
    ft_unique_lock<pt_mutex> result_guard;
    int lock_error;

    if (!base_config && !override_config)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    result = cnfg_config_create();
    if (!result)
        return (ft_nullptr);
    lock_error = cnfg_config_lock_if_enabled(result, result_guard);
    if (lock_error != ER_SUCCESS)
    {
        cnfg_config_unlock_guard(result_guard);
        cnfg_free(result);
        ft_errno = lock_error;
        return (ft_nullptr);
    }
    if (override_config && override_config->entry_count && !override_config->entries)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        cnfg_free(result);
        return (ft_nullptr);
    }
    if (base_config)
    {
        lock_error = cnfg_config_lock_if_enabled(const_cast<cnfg_config*>(base_config), base_guard);
        if (lock_error != ER_SUCCESS)
        {
            cnfg_config_unlock_guard(result_guard);
            cnfg_free(result);
            ft_errno = lock_error;
            return (ft_nullptr);
        }
    }
    if (override_config)
    {
        lock_error = cnfg_config_lock_if_enabled(const_cast<cnfg_config*>(override_config), override_guard);
        if (lock_error != ER_SUCCESS)
        {
            cnfg_config_unlock_guard(result_guard);
            cnfg_free(result);
            ft_errno = lock_error;
            return (ft_nullptr);
        }
    }
    if (config_copy_entries(result, base_config) != 0)
    {
        cnfg_config_unlock_guard(result_guard);
        cnfg_free(result);
        return (ft_nullptr);
    }
    if (!override_config)
    {
        ft_errno = ER_SUCCESS;
        return (result);
    }
    index = 0;
    while (index < override_config->entry_count)
    {
        const cnfg_entry *override_entry = &override_config->entries[index];
        cnfg_entry *existing = config_find_matching_entry(result, override_entry);
        if (existing)
        {
            cnfg_entry replacement;

            if (config_duplicate_entry(override_entry, &replacement) != 0)
            {
                cnfg_config_unlock_guard(result_guard);
                cnfg_free(result);
                return (ft_nullptr);
            }
            config_free_entry_contents(existing);
            *existing = replacement;
        }
        else
        {
            if (config_append_entry(result, override_entry) != 0)
            {
                cnfg_config_unlock_guard(result_guard);
                cnfg_free(result);
                return (ft_nullptr);
            }
        }
        ++index;
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

