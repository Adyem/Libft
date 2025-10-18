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

static void config_free_entry_contents_unlocked(cnfg_entry *entry)
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

static void config_free_entry_contents(cnfg_entry *entry)
{
    bool entry_locked;
    int entry_errno;

    if (!entry)
        return ;
    entry_errno = ft_errno;
    if (cnfg_entry_lock(entry, &entry_locked) != 0)
        return ;
    config_free_entry_contents_unlocked(entry);
    cnfg_entry_unlock(entry, entry_locked);
    ft_errno = entry_errno;
    return ;
}

static int config_duplicate_entry(const cnfg_entry *source, cnfg_entry *destination)
{
    cnfg_entry *mutable_source;
    bool source_locked;
    int entry_errno;

    if (!source || !destination)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    destination->mutex = ft_nullptr;
    destination->thread_safe_enabled = false;
    destination->section = ft_nullptr;
    destination->key = ft_nullptr;
    destination->value = ft_nullptr;
    mutable_source = const_cast<cnfg_entry*>(source);
    entry_errno = ft_errno;
    if (cnfg_entry_lock(mutable_source, &source_locked) != 0)
        return (-1);
    if (source->section)
    {
        destination->section = cma_strdup(source->section);
        if (!destination->section)
        {
            cnfg_entry_unlock(mutable_source, source_locked);
            ft_errno = entry_errno;
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    if (source->key)
    {
        destination->key = cma_strdup(source->key);
        if (!destination->key)
        {
            cnfg_entry_unlock(mutable_source, source_locked);
            ft_errno = entry_errno;
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    if (source->value)
    {
        destination->value = cma_strdup(source->value);
        if (!destination->value)
        {
            cnfg_entry_unlock(mutable_source, source_locked);
            ft_errno = entry_errno;
            config_free_entry_contents(destination);
            return (-1);
        }
    }
    cnfg_entry_unlock(mutable_source, source_locked);
    ft_errno = entry_errno;
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
    int entry_errno;

    if (!config || !entry)
        return (ft_nullptr);
    index = 0;
    while (index < config->entry_count)
    {
        cnfg_entry *candidate = &config->entries[index];
        bool candidate_locked;
        bool matches;

        entry_errno = ft_errno;
        if (cnfg_entry_lock(candidate, &candidate_locked) != 0)
            return (ft_nullptr);
        matches = config_strings_equal(candidate->section, entry->section)
            && config_strings_equal(candidate->key, entry->key);
        cnfg_entry_unlock(candidate, candidate_locked);
        ft_errno = entry_errno;
        if (matches)
            return (candidate);
        ++index;
    }
    return (ft_nullptr);
}

static int config_append_entry(cnfg_config *destination, const cnfg_entry *source)
{
    cnfg_entry copy;
    cnfg_entry *new_entries;
    cnfg_entry *target;

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
    target = &destination->entries[destination->entry_count];
    target->mutex = ft_nullptr;
    target->thread_safe_enabled = false;
    target->section = copy.section;
    target->key = copy.key;
    target->value = copy.value;
    copy.section = ft_nullptr;
    copy.key = ft_nullptr;
    copy.value = ft_nullptr;
    if (cnfg_entry_prepare_thread_safety(target) != 0)
    {
        config_free_entry_contents_unlocked(target);
        return (-1);
    }
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
            bool existing_locked;
            int entry_errno;

            if (config_duplicate_entry(override_entry, &replacement) != 0)
            {
                cnfg_config_unlock_guard(result_guard);
                cnfg_free(result);
                return (ft_nullptr);
            }
            entry_errno = ft_errno;
            if (cnfg_entry_lock(existing, &existing_locked) != 0)
            {
                config_free_entry_contents(&replacement);
                cnfg_config_unlock_guard(result_guard);
                cnfg_free(result);
                return (ft_nullptr);
            }
            config_free_entry_contents_unlocked(existing);
            existing->section = replacement.section;
            existing->key = replacement.key;
            existing->value = replacement.value;
            replacement.section = ft_nullptr;
            replacement.key = ft_nullptr;
            replacement.value = ft_nullptr;
            cnfg_entry_unlock(existing, existing_locked);
            ft_errno = entry_errno;
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

