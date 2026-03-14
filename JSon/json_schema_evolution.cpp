#include "json_schema_evolution.hpp"
#include "document.hpp"
#include "../Errno/errno.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../PThread/mutex.hpp"

struct json_schema_migration_step
{
    int32_t from_version;
    int32_t to_version;
    ft_function<int32_t(json_document &)> callback;
};

static ft_map<ft_string, ft_vector<json_schema_migration_step> > &json_schema_registry()
{
    static ft_map<ft_string, ft_vector<json_schema_migration_step> > registry;
    return (registry);
}

static pt_mutex &json_schema_registry_mutex()
{
    static pt_mutex mutex;
    return (mutex);
}

static int32_t json_schema_registry_lock(pt_mutex &mutex, ft_bool *lock_acquired) noexcept
{
    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    static ft_bool mutex_initialised = FT_FALSE;
    if (!mutex_initialised)
    {
        int32_t initialize_error = mutex.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
        mutex_initialised = FT_TRUE;
    }
    int32_t lock_error = mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

static int32_t json_schema_registry_unlock(pt_mutex &mutex, ft_bool lock_acquired) noexcept
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESS);
    return (mutex.unlock());
}

static int32_t json_schema_validate_identifier(const ft_string &schema_name) noexcept
{
    if (schema_name.size() == FT_ERR_SUCCESS)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_size_t index;
    ft_size_t length;
    const char *characters;

    index = 0;
    length = schema_name.size();
    characters = schema_name.data();
    if (!characters)
        return (FT_ERR_INVALID_ARGUMENT);
    while (index < length)
    {
        char current_character = characters[index];
        if ((current_character >= 'a' && current_character <= 'z')
            || (current_character >= 'A' && current_character <= 'Z')
            || (current_character >= '0' && current_character <= '9')
            || current_character == '_' || current_character == '-' || current_character == '.')
        {
            index++;
            continue ;
        }
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (FT_ERR_SUCCESS);
}

int32_t json_register_schema_migration(const ft_string &schema_name,
    int32_t from_version,
    int32_t to_version,
    const ft_function<int32_t(json_document &)> &callback) noexcept
{
    int32_t validate_error = json_schema_validate_identifier(schema_name);
    if (validate_error != FT_ERR_SUCCESS)
        return (validate_error);
    if (from_version < 0 || to_version <= from_version)
        return (FT_ERR_INVALID_ARGUMENT);
    if (!callback)
        return (FT_ERR_INVALID_ARGUMENT);
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_error = json_schema_registry_lock(mutex, &lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    json_schema_migration_step step;

    step.from_version = from_version;
    step.to_version = to_version;
    step.callback = callback;
    if (!entry)
    {
        ft_vector<json_schema_migration_step> steps;

        steps.push_back(step);
        int32_t vector_error = static_cast<int32_t>(steps.get_error());
        if (vector_error != FT_ERR_SUCCESS)
        {
            (void)json_schema_registry_unlock(mutex, lock_acquired);
            return (vector_error);
        }
        registry.insert(schema_name, ft_move(steps));
        Pair<ft_string, ft_vector<json_schema_migration_step> > *inserted_entry = registry.find(schema_name);
        (void)json_schema_registry_unlock(mutex, lock_acquired);
        if (!inserted_entry)
        {
            return (FT_ERR_NO_MEMORY);
        }
        return (FT_ERR_SUCCESS);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    ft_size_t steps_count = steps.size();
    ft_size_t index = 0;
    while (index < steps_count)
    {
        json_schema_migration_step &existing_step = steps[index];
        if (existing_step.from_version == from_version)
        {
            (void)json_schema_registry_unlock(mutex, lock_acquired);
            return (FT_ERR_ALREADY_EXISTS);
        }
        index++;
    }
    steps.push_back(step);
    int32_t vector_error = static_cast<int32_t>(steps.get_error());
    (void)json_schema_registry_unlock(mutex, lock_acquired);
    if (vector_error != FT_ERR_SUCCESS)
        return (vector_error);
    return (FT_ERR_SUCCESS);
}

int32_t json_apply_schema_migrations(json_document &document,
    const ft_string &schema_name,
    int32_t current_version,
    int32_t target_version) noexcept
{
    int32_t validate_error = json_schema_validate_identifier(schema_name);
    if (validate_error != FT_ERR_SUCCESS)
        return (validate_error);
    if (current_version < 0 || target_version < current_version)
        return (FT_ERR_INVALID_ARGUMENT);
    if (current_version == target_version)
        return (FT_ERR_SUCCESS);
    int32_t working_version = current_version;
    while (working_version < target_version)
    {
        pt_mutex &mutex = json_schema_registry_mutex();
        ft_bool lock_acquired = FT_FALSE;
        int32_t lock_error = json_schema_registry_lock(mutex, &lock_acquired);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
        Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
        if (!entry)
        {
            (void)json_schema_registry_unlock(mutex, lock_acquired);
            return (FT_ERR_NOT_FOUND);
        }
        ft_vector<json_schema_migration_step> &steps = entry->value;
        ft_size_t steps_count = steps.size();
        ft_size_t index = 0;
        ft_bool found = FT_FALSE;
        json_schema_migration_step selected_step;
        while (index < steps_count)
        {
            json_schema_migration_step &candidate = steps[index];
            if (candidate.from_version == working_version)
            {
                if (!found || candidate.to_version < selected_step.to_version)
                {
                    selected_step = candidate;
                    found = FT_TRUE;
                }
            }
            index++;
        }
        if (!found)
        {
            (void)json_schema_registry_unlock(mutex, lock_acquired);
            return (FT_ERR_NOT_FOUND);
        }
        int32_t next_version = selected_step.to_version;
        ft_function<int32_t(json_document &)> callback = selected_step.callback;
        (void)json_schema_registry_unlock(mutex, lock_acquired);
        if (!callback)
            return (FT_ERR_INVALID_STATE);
        int32_t callback_result = callback(document);
        if (callback_result != FT_ERR_SUCCESS)
            return (callback_result);
        working_version = next_version;
    }
    return (FT_ERR_SUCCESS);
}

int32_t json_get_latest_schema_version(const ft_string &schema_name,
    int32_t *out_version) noexcept
{
    if (!out_version)
        return (FT_ERR_INVALID_POINTER);
    int32_t validate_error = json_schema_validate_identifier(schema_name);
    if (validate_error != FT_ERR_SUCCESS)
        return (validate_error);
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_error = json_schema_registry_lock(mutex, &lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    if (!entry)
    {
        (void)json_schema_registry_unlock(mutex, lock_acquired);
        return (FT_ERR_NOT_FOUND);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    ft_size_t steps_count = steps.size();
    ft_size_t index = 0;
    ft_bool found = FT_FALSE;
    int32_t latest_version = 0;
    while (index < steps_count)
    {
        json_schema_migration_step &candidate = steps[index];
        if (!found || candidate.to_version > latest_version)
        {
            latest_version = candidate.to_version;
            found = FT_TRUE;
        }
        index++;
    }
    (void)json_schema_registry_unlock(mutex, lock_acquired);
    if (!found)
        return (FT_ERR_NOT_FOUND);
    *out_version = latest_version;
    return (FT_ERR_SUCCESS);
}
