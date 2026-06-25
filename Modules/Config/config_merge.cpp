#include "config.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Advanced/advanced.hpp"
#include <cstdint>
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

static void config_add_lock_target(const config_data **lock_targets,
    ft_size_t *lock_count, const config_data *candidate_target)
{
    ft_size_t index;

    if (!lock_targets || !lock_count || !candidate_target)
        return ;
    index = 0;
    while (index < *lock_count)
    {
        if (lock_targets[index] == candidate_target)
            return ;
        ++index;
    }
    lock_targets[*lock_count] = candidate_target;
    *lock_count = *lock_count + 1;
    return ;
}

static void config_sort_lock_targets(const config_data **lock_targets,
    ft_size_t lock_count)
{
    ft_size_t index_outer;

    if (!lock_targets)
        return ;
    index_outer = 0;
    while (index_outer < lock_count)
    {
        ft_size_t index_inner;

        index_inner = index_outer + 1;
        while (index_inner < lock_count)
        {
            if (reinterpret_cast<uintptr_t>(lock_targets[index_inner])
                < reinterpret_cast<uintptr_t>(lock_targets[index_outer]))
            {
                const config_data *temporary_target;

                temporary_target = lock_targets[index_outer];
                lock_targets[index_outer] = lock_targets[index_inner];
                lock_targets[index_inner] = temporary_target;
            }
            ++index_inner;
        }
        ++index_outer;
    }
    return ;
}

static void config_unlock_targets_reverse(const config_data **lock_targets,
    ft_bool *lock_acquired, ft_size_t lock_count)
{
    if (!lock_targets || !lock_acquired)
        return ;
    while (lock_count > 0)
    {
        --lock_count;
        if (lock_acquired[lock_count] == FT_TRUE)
            config_unlock_guard(lock_targets[lock_count], FT_TRUE);
    }
    return ;
}

static int32_t config_lock_targets_in_order(const config_data **lock_targets,
    ft_bool *lock_acquired, ft_size_t lock_count)
{
    ft_size_t index;
    int32_t lock_error;

    if (!lock_targets || !lock_acquired)
        return (FT_ERR_INVALID_ARGUMENT);
    index = 0;
    while (index < lock_count)
    {
        lock_error = config_lock_if_enabled(lock_targets[index],
                &lock_acquired[index]);
        if (lock_error != FT_ERR_SUCCESS)
        {
            config_unlock_targets_reverse(lock_targets, lock_acquired, index);
            return (lock_error);
        }
        ++index;
    }
    return (FT_ERR_SUCCESS);
}

static void config_free_entry_contents_unlocked(config_entry *entry)
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

static void config_free_entry_contents(config_entry *entry)
{
    if (!entry)
    {
        return ;
    }
    ft_bool entry_locked;
    if (config_entry_lock(entry, &entry_locked) != FT_ERR_SUCCESS)
        return ;
    config_free_entry_contents_unlocked(entry);
    config_entry_unlock(entry, entry_locked);
    return ;
}

static int32_t config_duplicate_entry(const config_entry *source, config_entry *destination)
{
    config_entry *mutable_source;
    ft_bool source_locked;

    if (!source || !destination)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    destination->mutex = ft_nullptr;
    destination->section = ft_nullptr;
    destination->key = ft_nullptr;
    destination->value = ft_nullptr;
    mutable_source = const_cast<config_entry*>(source);
    if (config_entry_lock(mutable_source, &source_locked) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (source->section)
    {
        destination->section = adv_strdup(source->section);
        if (!destination->section)
        {
            config_entry_unlock(mutable_source, source_locked);
            config_free_entry_contents(destination);
            return (FT_ERR_NO_MEMORY);
        }
    }
    if (source->key)
    {
        destination->key = adv_strdup(source->key);
        if (!destination->key)
        {
            config_entry_unlock(mutable_source, source_locked);
            config_free_entry_contents(destination);
            return (FT_ERR_NO_MEMORY);
        }
    }
    if (source->value)
    {
        destination->value = adv_strdup(source->value);
        if (!destination->value)
        {
            config_entry_unlock(mutable_source, source_locked);
            config_free_entry_contents(destination);
            return (FT_ERR_NO_MEMORY);
        }
    }
    config_entry_unlock(mutable_source, source_locked);
    return (FT_ERR_SUCCESS);
}

static ft_bool config_strings_equal(const char *left, const char *right)
{
    if (!left && !right)
        return (FT_TRUE);
    if (!left || !right)
        return (FT_FALSE);
    if (ft_strcmp(left, right) == 0)
        return (FT_TRUE);
    return (FT_FALSE);
}

static config_entry *config_find_matching_entry(config_data *config, const config_entry *entry)
{
    ft_size_t index;

    if (!config || !entry)
    {
        return (ft_nullptr);
    }
    index = 0;
    while (index < config->entry_count)
    {
        config_entry *candidate = &config->entries[index];
        ft_bool candidate_locked;
        ft_bool matches;
        if (config_entry_lock(candidate, &candidate_locked) != FT_ERR_SUCCESS)
            return (ft_nullptr);
        matches = config_strings_equal(candidate->section, entry->section)
            && config_strings_equal(candidate->key, entry->key);
        config_entry_unlock(candidate, candidate_locked);
        if (matches)
        {
            return (candidate);
        }
        ++index;
    }
    return (ft_nullptr);
}

static int32_t config_append_entry(config_data *destination, const config_entry *source)
{
    config_entry copy;
    config_entry *new_entries;
    config_entry *target;

    if (!destination || !source)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (config_duplicate_entry(source, &copy) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    new_entries = static_cast<config_entry*>(cma_realloc(destination->entries,
            sizeof(config_entry) * (destination->entry_count + 1)));
    if (!new_entries)
    {
        config_free_entry_contents(&copy);
        return (FT_ERR_NO_MEMORY);
    }
    destination->entries = new_entries;
    target = &destination->entries[destination->entry_count];
    target->mutex = ft_nullptr;
    target->section = copy.section;
    target->key = copy.key;
    target->value = copy.value;
    copy.section = ft_nullptr;
    copy.key = ft_nullptr;
    copy.value = ft_nullptr;
    if (config_entry_prepare_thread_safety(target) != FT_ERR_SUCCESS)
    {
        config_free_entry_contents_unlocked(target);
        return (FT_ERR_INTERNAL);
    }
    destination->entry_count++;
    return (FT_ERR_SUCCESS);
}

static int32_t config_copy_entries(config_data *destination, const config_data *source)
{
    ft_size_t index;

    if (!source)
        return (FT_ERR_SUCCESS);
    if (source->entry_count && !source->entries)
    {
        return (FT_ERR_INVALID_STATE);
    }
    index = 0;
    while (index < source->entry_count)
    {
        if (config_append_entry(destination, &source->entries[index]) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        ++index;
    }
    return (FT_ERR_SUCCESS);
}

config_data *config_merge(const config_data *base_config, const config_data *override_config)
{
    config_data *result;
    ft_size_t index;
    const config_data *lock_targets[3];
    ft_bool lock_acquired[3];
    ft_size_t lock_count;
    int32_t lock_error;

    lock_acquired[0] = FT_FALSE;
    lock_acquired[1] = FT_FALSE;
    lock_acquired[2] = FT_FALSE;
    lock_count = 0;
    if (!base_config && !override_config)
    {
        return (ft_nullptr);
    }
    result = config_data_create();
    if (!result)
        return (ft_nullptr);
    config_add_lock_target(lock_targets, &lock_count, result);
    config_add_lock_target(lock_targets, &lock_count, base_config);
    config_add_lock_target(lock_targets, &lock_count, override_config);
    config_sort_lock_targets(lock_targets, lock_count);
    lock_error = config_lock_targets_in_order(lock_targets, lock_acquired,
            lock_count);
    if (lock_error != FT_ERR_SUCCESS)
    {
        config_data_free(result);
        return (ft_nullptr);
    }
    if (override_config && override_config->entry_count && !override_config->entries)
    {
        config_unlock_targets_reverse(lock_targets, lock_acquired, lock_count);
        config_data_free(result);
        return (ft_nullptr);
    }
    if (config_copy_entries(result, base_config) != FT_ERR_SUCCESS)
    {
        config_unlock_targets_reverse(lock_targets, lock_acquired, lock_count);
        config_data_free(result);
        return (ft_nullptr);
    }
    if (!override_config)
    {
        config_unlock_targets_reverse(lock_targets, lock_acquired, lock_count);
        return (result);
    }
    index = 0;
    while (index < override_config->entry_count)
    {
        const config_entry *override_entry = &override_config->entries[index];
        config_entry *existing = config_find_matching_entry(result, override_entry);
        if (existing)
        {
            config_entry replacement;
            ft_bool existing_locked;

            if (config_duplicate_entry(override_entry, &replacement) != FT_ERR_SUCCESS)
            {
                config_unlock_targets_reverse(lock_targets, lock_acquired,
                    lock_count);
                config_data_free(result);
                return (ft_nullptr);
            }
            if (config_entry_lock(existing, &existing_locked) != FT_ERR_SUCCESS)
            {
                config_free_entry_contents(&replacement);
                config_unlock_targets_reverse(lock_targets, lock_acquired,
                    lock_count);
                config_data_free(result);
                return (ft_nullptr);
            }
            config_free_entry_contents_unlocked(existing);
            existing->section = replacement.section;
            existing->key = replacement.key;
            existing->value = replacement.value;
            replacement.section = ft_nullptr;
            replacement.key = ft_nullptr;
            replacement.value = ft_nullptr;
            config_entry_unlock(existing, existing_locked);
        }
        else
        {
            if (config_append_entry(result, override_entry) != FT_ERR_SUCCESS)
            {
                config_unlock_targets_reverse(lock_targets, lock_acquired,
                    lock_count);
                config_data_free(result);
                return (ft_nullptr);
            }
        }
        ++index;
    }
    config_unlock_targets_reverse(lock_targets, lock_acquired, lock_count);
    return (result);
}
