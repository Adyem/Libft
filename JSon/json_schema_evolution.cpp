#include "json_schema_evolution.hpp"
#include "document.hpp"
#include "../Errno/errno.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

struct json_schema_migration_step
{
    int from_version;
    int to_version;
    ft_function<int(json_document &)> callback;
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

static void json_schema_unlock(ft_unique_lock<pt_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

static bool json_schema_validate_identifier(const ft_string &schema_name) noexcept
{
    if (schema_name.size() == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    size_t index;
    size_t length;
    const char *characters;

    index = 0;
    length = schema_name.size();
    characters = schema_name.data();
    if (!characters)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

int json_register_schema_migration(const ft_string &schema_name,
    int from_version,
    int to_version,
    const ft_function<int(json_document &)> &callback) noexcept
{
    if (!json_schema_validate_identifier(schema_name))
        return (ft_errno);
    if (from_version < 0 || to_version <= from_version)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (!callback)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_unique_lock<pt_mutex> guard(mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        int lock_error = guard.get_error();
        json_schema_unlock(guard);
        ft_errno = lock_error;
        return (lock_error);
    }
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    int map_error = registry.get_error();
    if (map_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        ft_errno = map_error;
        return (map_error);
    }
    json_schema_migration_step step;

    step.from_version = from_version;
    step.to_version = to_version;
    step.callback = callback;
    int callback_error = step.callback.get_error();
    if (callback_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        ft_errno = callback_error;
        return (callback_error);
    }
    if (!entry)
    {
        ft_vector<json_schema_migration_step> steps;

        steps.push_back(step);
        if (steps.get_error() != FT_ERR_SUCCESSS)
        {
            int vector_error = steps.get_error();
            json_schema_unlock(guard);
            ft_errno = vector_error;
            return (vector_error);
        }
        registry.insert(schema_name, ft_move(steps));
        map_error = registry.get_error();
        json_schema_unlock(guard);
        if (map_error != FT_ERR_SUCCESSS)
        {
            ft_errno = map_error;
            return (map_error);
        }
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    size_t steps_count = steps.size();
    int vector_error = steps.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        ft_errno = vector_error;
        return (vector_error);
    }
    size_t index = 0;
    while (index < steps_count)
    {
        json_schema_migration_step &existing_step = steps[index];
        if (existing_step.from_version == from_version)
        {
            json_schema_unlock(guard);
            ft_errno = FT_ERR_ALREADY_EXISTS;
            return (FT_ERR_ALREADY_EXISTS);
        }
        index++;
    }
    steps.push_back(step);
    vector_error = steps.get_error();
    json_schema_unlock(guard);
    if (vector_error != FT_ERR_SUCCESSS)
    {
        ft_errno = vector_error;
        return (vector_error);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

int json_apply_schema_migrations(json_document &document,
    const ft_string &schema_name,
    int current_version,
    int target_version) noexcept
{
    if (!json_schema_validate_identifier(schema_name))
        return (ft_errno);
    if (current_version < 0 || target_version < current_version)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (current_version == target_version)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    int working_version = current_version;
    while (working_version < target_version)
    {
        pt_mutex &mutex = json_schema_registry_mutex();
        ft_unique_lock<pt_mutex> guard(mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            int lock_error = guard.get_error();
            json_schema_unlock(guard);
            ft_errno = lock_error;
            return (lock_error);
        }
        ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
        Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
        int map_error = registry.get_error();
        if (map_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            ft_errno = map_error;
            return (map_error);
        }
        if (!entry)
        {
            json_schema_unlock(guard);
            ft_errno = FT_ERR_NOT_FOUND;
            return (FT_ERR_NOT_FOUND);
        }
        ft_vector<json_schema_migration_step> &steps = entry->value;
        size_t steps_count = steps.size();
        int vector_error = steps.get_error();
        if (vector_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            ft_errno = vector_error;
            return (vector_error);
        }
        size_t index = 0;
        bool found = false;
        json_schema_migration_step selected_step;
        while (index < steps_count)
        {
            json_schema_migration_step &candidate = steps[index];
            if (candidate.from_version == working_version)
            {
                if (!found || candidate.to_version < selected_step.to_version)
                {
                    selected_step = candidate;
                    found = true;
                }
            }
            index++;
        }
        if (!found)
        {
            json_schema_unlock(guard);
            ft_errno = FT_ERR_NOT_FOUND;
            return (FT_ERR_NOT_FOUND);
        }
        int next_version = selected_step.to_version;
        ft_function<int(json_document &)> callback = selected_step.callback;
        int callback_error = callback.get_error();
        if (callback_error != FT_ERR_SUCCESSS)
        {
            ft_errno = callback_error;
            return (callback_error);
        }
        json_schema_unlock(guard);
        if (!callback)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (FT_ERR_INVALID_STATE);
        }
        int callback_result = callback(document);
        if (callback_result != FT_ERR_SUCCESSS)
        {
            ft_errno = callback_result;
            return (callback_result);
        }
        working_version = next_version;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

int json_get_latest_schema_version(const ft_string &schema_name,
    int *out_version) noexcept
{
    if (!out_version)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (FT_ERR_INVALID_POINTER);
    }
    if (!json_schema_validate_identifier(schema_name))
        return (ft_errno);
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_unique_lock<pt_mutex> guard(mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        int lock_error = guard.get_error();
        json_schema_unlock(guard);
        ft_errno = lock_error;
        return (lock_error);
    }
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    int map_error = registry.get_error();
    if (map_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        ft_errno = map_error;
        return (map_error);
    }
    if (!entry)
    {
        json_schema_unlock(guard);
        ft_errno = FT_ERR_NOT_FOUND;
        return (FT_ERR_NOT_FOUND);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    size_t steps_count = steps.size();
    int vector_error = steps.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        ft_errno = vector_error;
        return (vector_error);
    }
    size_t index = 0;
    bool found = false;
    int latest_version = 0;
    while (index < steps_count)
    {
        json_schema_migration_step &candidate = steps[index];
        if (!found || candidate.to_version > latest_version)
        {
            latest_version = candidate.to_version;
            found = true;
        }
        index++;
    }
    json_schema_unlock(guard);
    if (!found)
    {
        ft_errno = FT_ERR_NOT_FOUND;
        return (FT_ERR_NOT_FOUND);
    }
    *out_version = latest_version;
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}
