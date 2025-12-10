#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>
#include <utility>
#include "../Template/move.hpp"

static pt_mutex g_env_mutex;

static int su_environment_unlock_with_error(int error_code)
{
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    ft_errno = error_code;
    return (-1);
}

static int su_environment_snapshot_contains(
    const t_su_environment_snapshot *snapshot,
    const ft_string &name,
    int *is_present)
{
    size_t  snapshot_count;
    size_t  index;
    size_t  target_length;

    if (snapshot == ft_nullptr || is_present == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    snapshot_count = snapshot->entries.size();
    if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = snapshot->entries.get_error();
        return (-1);
    }
    target_length = name.size();
    if (name.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = name.get_error();
        return (-1);
    }
    index = 0;
    while (index < snapshot_count)
    {
        const ft_string &entry = snapshot->entries[index];
        const char      *entry_data;
        const char      *equals_location;
        size_t          entry_length;

        if (entry.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = entry.get_error();
            return (-1);
        }
        entry_data = entry.c_str();
        equals_location = ft_strchr(entry_data, '=');
        if (equals_location != ft_nullptr)
            entry_length = static_cast<size_t>(equals_location - entry_data);
        else
            entry_length = ft_strlen_size_t(entry_data);
        if (entry_length == target_length
            && ft_strncmp(entry_data, name.c_str(), target_length) == 0)
        {
            *is_present = 1;
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        index += 1;
    }
    *is_present = 0;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

static int su_environment_split_entry(
    const char *entry,
    ft_string &name,
    ft_string &value,
    int *has_value)
{
    const char  *equals_location;
    size_t      name_length;

    if (entry == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    equals_location = ft_strchr(entry, '=');
    if (equals_location == ft_nullptr)
    {
        name.assign(entry, ft_strlen_size_t(entry));
        if (name.get_error() != FT_ER_SUCCESSS)
            return (-1);
        value.clear();
        if (value.get_error() != FT_ER_SUCCESSS)
            return (-1);
        if (has_value != ft_nullptr)
            *has_value = 0;
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    name_length = static_cast<size_t>(equals_location - entry);
    name.assign(entry, name_length);
    if (name.get_error() != FT_ER_SUCCESSS)
        return (-1);
    value.assign(equals_location + 1,
        ft_strlen_size_t(equals_location + 1));
    if (value.get_error() != FT_ER_SUCCESSS)
        return (-1);
    if (has_value != ft_nullptr)
        *has_value = 1;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

char    *su_getenv(const char *name)
{
    char    *result;

    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_nullptr);
    result = ft_getenv(name);
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (ft_nullptr);
    return (result);
}

int su_setenv(const char *name, const char *value, int overwrite)
{
    int result;

    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    result = ft_setenv(name, value, overwrite);
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    return (result);
}

int su_putenv(char *string)
{
    int result;
    int cmp_error_code;

    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    cmp_error_code = FT_ER_SUCCESSS;
    result = cmp_putenv(string);
    if (result != 0)
        cmp_error_code = ft_errno;
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    if (result == 0)
        ft_errno = FT_ER_SUCCESSS;
    else
        ft_errno = cmp_error_code;
    return (result);
}

int su_environment_snapshot_capture(t_su_environment_snapshot *snapshot)
{
    char    **environment_entries;
    size_t  index;

    if (snapshot == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    snapshot->entries.clear();
    if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = snapshot->entries.get_error();
        return (-1);
    }
    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    environment_entries = cmp_get_environ_entries();
    index = 0;
    if (environment_entries != ft_nullptr)
    {
        while (environment_entries[index] != ft_nullptr)
        {
            ft_string entry_copy(environment_entries[index]);

            if (entry_copy.get_error() != FT_ER_SUCCESSS)
                return (su_environment_unlock_with_error(entry_copy.get_error()));
            snapshot->entries.push_back(ft_move(entry_copy));
            if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
                return (su_environment_unlock_with_error(snapshot->entries.get_error()));
            index += 1;
        }
    }
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int su_environment_snapshot_restore(const t_su_environment_snapshot *snapshot)
{
    ft_vector<ft_string>    current_environment(4);
    char                    **environment_entries;
    size_t                  index;

    if (snapshot == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = snapshot->entries.get_error();
        return (-1);
    }
    if (g_env_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    environment_entries = cmp_get_environ_entries();
    index = 0;
    if (environment_entries != ft_nullptr)
    {
        while (environment_entries[index] != ft_nullptr)
        {
            ft_string entry_copy(environment_entries[index]);

            if (entry_copy.get_error() != FT_ER_SUCCESSS)
                return (su_environment_unlock_with_error(entry_copy.get_error()));
            current_environment.push_back(ft_move(entry_copy));
            if (current_environment.get_error() != FT_ER_SUCCESSS)
                return (su_environment_unlock_with_error(current_environment.get_error()));
            index += 1;
        }
    }
    size_t current_count = current_environment.size();
    if (current_environment.get_error() != FT_ER_SUCCESSS)
        return (su_environment_unlock_with_error(current_environment.get_error()));
    index = 0;
    while (index < current_count)
    {
        ft_string &entry = current_environment[index];
        ft_string name;
        ft_string value;
        int       is_present;

        if (entry.get_error() != FT_ER_SUCCESSS)
            return (su_environment_unlock_with_error(entry.get_error()));
        if (su_environment_split_entry(entry.c_str(), name, value, ft_nullptr) != 0)
            return (su_environment_unlock_with_error(ft_errno));
        if (name.get_error() != FT_ER_SUCCESSS)
            return (su_environment_unlock_with_error(name.get_error()));
        if (su_environment_snapshot_contains(snapshot, name, &is_present) != 0)
            return (su_environment_unlock_with_error(ft_errno));
        if (is_present == 0)
        {
            if (ft_unsetenv(name.c_str()) != 0)
                return (su_environment_unlock_with_error(ft_errno));
        }
        index += 1;
    }
    size_t snapshot_count = snapshot->entries.size();
    if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
        return (su_environment_unlock_with_error(snapshot->entries.get_error()));
    index = 0;
    while (index < snapshot_count)
    {
        const ft_string &entry = snapshot->entries[index];
        ft_string        name;
        ft_string        value;
        int              has_value;

        if (entry.get_error() != FT_ER_SUCCESSS)
            return (su_environment_unlock_with_error(entry.get_error()));
        if (su_environment_split_entry(entry.c_str(), name, value, &has_value) != 0)
            return (su_environment_unlock_with_error(ft_errno));
        if (name.get_error() != FT_ER_SUCCESSS)
            return (su_environment_unlock_with_error(name.get_error()));
        if (has_value != 0)
        {
            if (ft_setenv(name.c_str(), value.c_str(), 1) != 0)
                return (su_environment_unlock_with_error(ft_errno));
        }
        else
        {
            if (ft_unsetenv(name.c_str()) != 0)
                return (su_environment_unlock_with_error(ft_errno));
        }
        index += 1;
    }
    if (g_env_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

void su_environment_snapshot_dispose(t_su_environment_snapshot *snapshot)
{
    if (snapshot == ft_nullptr)
        return ;
    snapshot->entries.clear();
    if (snapshot->entries.get_error() != FT_ER_SUCCESSS)
        ft_errno = snapshot->entries.get_error();
    else
        ft_errno = FT_ER_SUCCESSS;
    return ;
}

int su_environment_sandbox_begin(t_su_environment_snapshot *snapshot)
{
    return (su_environment_snapshot_capture(snapshot));
}

int su_environment_sandbox_end(t_su_environment_snapshot *snapshot)
{
    int restore_result;
    int restore_errno;

    restore_result = su_environment_snapshot_restore(snapshot);
    restore_errno = ft_errno;
    su_environment_snapshot_dispose(snapshot);
    if (restore_result != 0)
    {
        ft_errno = restore_errno;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
