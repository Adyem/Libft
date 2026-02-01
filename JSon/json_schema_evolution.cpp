#include "json_schema_evolution.hpp"
#include "document.hpp"
#include "../Errno/errno.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Template/move.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/lock_error_helpers.hpp"

static void json_schema_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

#define JSON_SCHEMA_ERROR_RETURN(code, value) \
    do { json_schema_push_error(code); return (value); } while (0)

#define JSON_SCHEMA_SUCCESS_RETURN(value) \
    do { json_schema_push_error(FT_ERR_SUCCESSS); return (value); } while (0)

static int json_schema_last_error(void)
{
    return (ft_global_error_stack_last_error());
}

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
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, false);
    }
    size_t index;
    size_t length;
    const char *characters;

    index = 0;
    length = schema_name.size();
    characters = schema_name.data();
    if (!characters)
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, false);
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
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, false);
    }
    JSON_SCHEMA_SUCCESS_RETURN(true);
}

int json_register_schema_migration(const ft_string &schema_name,
    int from_version,
    int to_version,
    const ft_function<int(json_document &)> &callback) noexcept
{
    if (!json_schema_validate_identifier(schema_name))
        return (json_schema_last_error());
    if (from_version < 0 || to_version <= from_version)
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, FT_ERR_INVALID_ARGUMENT);
    if (!callback)
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, FT_ERR_INVALID_ARGUMENT);
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_unique_lock<pt_mutex> guard(mutex);
    {
        int lock_error = ft_unique_lock_pop_last_error(guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(lock_error, lock_error);
        }
    }
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    int map_error = registry.pop_newest_operation_error();
    if (map_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(map_error, map_error);
    }
    json_schema_migration_step step;

    step.from_version = from_version;
    step.to_version = to_version;
    step.callback = callback;
    if (!entry)
    {
        ft_vector<json_schema_migration_step> steps;

        steps.push_back(step);
        if (steps.get_error() != FT_ERR_SUCCESSS)
        {
            int vector_error = steps.get_error();
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(vector_error, vector_error);
        }
        registry.insert(schema_name, ft_move(steps));
        map_error = registry.pop_newest_operation_error();
        json_schema_unlock(guard);
        if (map_error != FT_ERR_SUCCESSS)
        {
            JSON_SCHEMA_ERROR_RETURN(map_error, map_error);
        }
        JSON_SCHEMA_SUCCESS_RETURN(FT_ERR_SUCCESSS);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    size_t steps_count = steps.size();
    int vector_error = steps.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(vector_error, vector_error);
    }
    size_t index = 0;
    while (index < steps_count)
    {
        json_schema_migration_step &existing_step = steps[index];
        if (existing_step.from_version == from_version)
        {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_ALREADY_EXISTS, FT_ERR_ALREADY_EXISTS);
        }
        index++;
    }
    steps.push_back(step);
    vector_error = steps.get_error();
    json_schema_unlock(guard);
    if (vector_error != FT_ERR_SUCCESSS)
        JSON_SCHEMA_ERROR_RETURN(vector_error, vector_error);
    JSON_SCHEMA_SUCCESS_RETURN(FT_ERR_SUCCESSS);
}

int json_apply_schema_migrations(json_document &document,
    const ft_string &schema_name,
    int current_version,
    int target_version) noexcept
{
    if (!json_schema_validate_identifier(schema_name))
        return (json_schema_last_error());
    if (current_version < 0 || target_version < current_version)
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, FT_ERR_INVALID_ARGUMENT);
    if (current_version == target_version)
        JSON_SCHEMA_SUCCESS_RETURN(FT_ERR_SUCCESSS);
    int working_version = current_version;
    while (working_version < target_version)
    {
        pt_mutex &mutex = json_schema_registry_mutex();
        ft_unique_lock<pt_mutex> guard(mutex);
        {
            int lock_error = ft_unique_lock_pop_last_error(guard);

            if (lock_error != FT_ERR_SUCCESSS)
            {
                json_schema_unlock(guard);
                JSON_SCHEMA_ERROR_RETURN(lock_error, lock_error);
            }
        }
        ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
        Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
        int map_error = registry.pop_newest_operation_error();
        if (map_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(map_error, map_error);
        }
        if (!entry)
        {
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(FT_ERR_NOT_FOUND, FT_ERR_NOT_FOUND);
        }
        ft_vector<json_schema_migration_step> &steps = entry->value;
        size_t steps_count = steps.size();
        int vector_error = steps.get_error();
        if (vector_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(vector_error, vector_error);
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
            JSON_SCHEMA_ERROR_RETURN(FT_ERR_NOT_FOUND, FT_ERR_NOT_FOUND);
        }
        int next_version = selected_step.to_version;
        ft_function<int(json_document &)> callback = selected_step.callback;
        json_schema_unlock(guard);
        if (!callback)
            JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_STATE, FT_ERR_INVALID_STATE);
        int callback_result = callback(document);
        if (callback_result != FT_ERR_SUCCESSS)
            JSON_SCHEMA_ERROR_RETURN(callback_result, callback_result);
        working_version = next_version;
    }
    JSON_SCHEMA_SUCCESS_RETURN(FT_ERR_SUCCESSS);
}

int json_get_latest_schema_version(const ft_string &schema_name,
    int *out_version) noexcept
{
    if (!out_version)
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_INVALID_POINTER, FT_ERR_INVALID_POINTER);
    if (!json_schema_validate_identifier(schema_name))
        return (json_schema_last_error());
    pt_mutex &mutex = json_schema_registry_mutex();
    ft_unique_lock<pt_mutex> guard(mutex);
    {
        int lock_error = ft_unique_lock_pop_last_error(guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            json_schema_unlock(guard);
            JSON_SCHEMA_ERROR_RETURN(lock_error, lock_error);
        }
    }
    ft_map<ft_string, ft_vector<json_schema_migration_step> > &registry = json_schema_registry();
    Pair<ft_string, ft_vector<json_schema_migration_step> > *entry = registry.find(schema_name);
    int map_error = registry.pop_newest_operation_error();
    if (map_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(map_error, map_error);
    }
    if (!entry)
    {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_NOT_FOUND, FT_ERR_NOT_FOUND);
    }
    ft_vector<json_schema_migration_step> &steps = entry->value;
    size_t steps_count = steps.size();
    int vector_error = steps.get_error();
    if (vector_error != FT_ERR_SUCCESSS)
    {
        json_schema_unlock(guard);
        JSON_SCHEMA_ERROR_RETURN(vector_error, vector_error);
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
        JSON_SCHEMA_ERROR_RETURN(FT_ERR_NOT_FOUND, FT_ERR_NOT_FOUND);
    *out_version = latest_version;
    JSON_SCHEMA_SUCCESS_RETURN(FT_ERR_SUCCESSS);
}
