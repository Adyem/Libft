#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>
#include <cerrno>
#include <utility>
#include "../Template/move.hpp"

static pthread_mutex_t *g_env_mutex = ft_nullptr;

static int su_environment_lock_mutex(void)
{
    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int lock_error = pt_pthread_mutex_lock_with_error(g_env_mutex);
    ft_global_error_stack_drop_last_error();
    return (lock_error);
}

static int su_environment_unlock_mutex(void)
{
    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int unlock_error = pt_pthread_mutex_unlock_with_error(g_env_mutex);
    ft_global_error_stack_drop_last_error();
    return (unlock_error);
}

int su_environment_enable_thread_safety(void)
{
    if (g_env_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pthread_mutex_t *mutex_pointer = static_cast<pthread_mutex_t*>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    int pthread_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
    if (pthread_error != 0)
    {
        int error_code = ft_map_system_error(pthread_error);
        std::free(mutex_pointer);
        ft_global_error_stack_push(error_code);
        return (error_code);
    }
    g_env_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void su_environment_disable_thread_safety(void)
{
    if (g_env_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    int pthread_error = pthread_mutex_destroy(g_env_mutex);
    std::free(g_env_mutex);
    g_env_mutex = ft_nullptr;
    if (pthread_error != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(pthread_error));
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

static int g_environment_thread_safety_initializer = su_environment_enable_thread_safety();

static int su_environment_unlock_with_error(int error_code)
{
    if (g_env_mutex == ft_nullptr)
        return (error_code);
    int unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    return (error_code);
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
        return (FT_ERR_INVALID_ARGUMENT);
    }
    snapshot_count = snapshot->entries.size();
    if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
    {
        return (snapshot->entries.get_error());
    }
    target_length = name.size();
    {
        int    string_error;

        string_error = name.pop_operation_error(name.last_operation_id());
        if (string_error != FT_ERR_SUCCESSS)
        {
            return (string_error);
        }
    }
    index = 0;
    while (index < snapshot_count)
    {
        const ft_string &entry = snapshot->entries[index];
        const char      *entry_data;
        const char      *equals_location;
        size_t          entry_length;

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
            return (FT_ERR_SUCCESSS);
        }
        index += 1;
    }
    *is_present = 0;
    return (FT_ERR_SUCCESSS);
}

static int su_environment_split_entry(
    const char *entry,
    ft_string &name,
    ft_string &value,
    int *has_value)
{
    const char  *equals_location;
    size_t      name_length;
    int         string_error;

    if (entry == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    equals_location = ft_strchr(entry, '=');
    if (equals_location == ft_nullptr)
    {
        name.assign(entry, ft_strlen_size_t(entry));
        string_error = name.pop_operation_error(name.last_operation_id());
        if (string_error != FT_ERR_SUCCESSS)
            return (string_error);
        value.clear();
        string_error = value.pop_operation_error(value.last_operation_id());
        if (string_error != FT_ERR_SUCCESSS)
            return (string_error);
        if (has_value != ft_nullptr)
            *has_value = 0;
        return (FT_ERR_SUCCESSS);
    }
    name_length = static_cast<size_t>(equals_location - entry);
    name.assign(entry, name_length);
    string_error = name.pop_operation_error(name.last_operation_id());
    if (string_error != FT_ERR_SUCCESSS)
        return (string_error);
    value.assign(equals_location + 1,
        ft_strlen_size_t(equals_location + 1));
    string_error = value.pop_operation_error(value.last_operation_id());
    if (string_error != FT_ERR_SUCCESSS)
        return (string_error);
    if (has_value != ft_nullptr)
        *has_value = 1;
    return (FT_ERR_SUCCESSS);
}

char    *su_getenv(const char *name)
{
    char    *result;
    int     error_code;
    int     mutex_error;

    mutex_error = su_environment_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (ft_nullptr);
    }
    result = ft_getenv(name);
    error_code = ft_global_error_stack_drop_last_error();
    mutex_error = su_environment_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (ft_nullptr);
    }
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

int su_setenv(const char *name, const char *value, int overwrite)
{
    int result;
    int error_code;
    int mutex_error;

    mutex_error = su_environment_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    result = ft_setenv(name, value, overwrite);
    error_code = ft_global_error_stack_drop_last_error();
    mutex_error = su_environment_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    if (result != 0)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

int su_putenv(char *string)
{
    int result;
    int error_code;
    int mutex_error;

    if (string == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutex_error = su_environment_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    errno = 0;
    result = cmp_putenv(string);
    if (result != 0)
    {
    }
    mutex_error = su_environment_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    error_code = cmp_last_error();
    if (result != 0)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

int su_environment_snapshot_capture(t_su_environment_snapshot *snapshot)
{
    char    **environment_entries;
    size_t  index;
    int     error_code;
    int     mutex_error;

    if (snapshot == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    snapshot->entries.clear();
    if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = snapshot->entries.get_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    mutex_error = su_environment_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    environment_entries = cmp_get_environ_entries();
    index = 0;
    if (environment_entries != ft_nullptr)
    {
        while (environment_entries[index] != ft_nullptr)
        {
            ft_string entry_copy(environment_entries[index]);

            {
                int string_error = entry_copy.pop_operation_error(
                    entry_copy.last_operation_id());

                if (string_error != FT_ERR_SUCCESSS)
                {
                    error_code = su_environment_unlock_with_error(string_error);
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
            }
            snapshot->entries.push_back(ft_move(entry_copy));
            if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
            {
                error_code = su_environment_unlock_with_error(snapshot->entries.get_error());
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            index += 1;
        }
    }
    mutex_error = su_environment_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_environment_snapshot_restore(const t_su_environment_snapshot *snapshot)
{
    ft_vector<ft_string>    current_environment(4);
    char                    **environment_entries;
    size_t                  index;
    int                     error_code;
    int                     mutex_error;

    if (snapshot == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = snapshot->entries.get_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    mutex_error = su_environment_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    environment_entries = cmp_get_environ_entries();
    index = 0;
    if (environment_entries != ft_nullptr)
    {
        while (environment_entries[index] != ft_nullptr)
        {
            ft_string entry_copy(environment_entries[index]);

            {
                int string_error = entry_copy.pop_operation_error(
                    entry_copy.last_operation_id());

                if (string_error != FT_ERR_SUCCESSS)
                {
                    error_code = su_environment_unlock_with_error(string_error);
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
            }
            current_environment.push_back(ft_move(entry_copy));
            if (current_environment.get_error() != FT_ERR_SUCCESSS)
            {
                error_code = su_environment_unlock_with_error(current_environment.get_error());
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            index += 1;
        }
    }
    size_t current_count = current_environment.size();
    if (current_environment.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = su_environment_unlock_with_error(current_environment.get_error());
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    index = 0;
    while (index < current_count)
    {
        ft_string &entry = current_environment[index];
        ft_string name;
        ft_string value;
        int       is_present;

        error_code = su_environment_split_entry(entry.c_str(), name, value, ft_nullptr);
        if (error_code != FT_ERR_SUCCESSS)
        {
            error_code = su_environment_unlock_with_error(error_code);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        error_code = su_environment_snapshot_contains(snapshot, name, &is_present);
        if (error_code != FT_ERR_SUCCESSS)
        {
            error_code = su_environment_unlock_with_error(error_code);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (is_present == 0)
        {
            if (ft_unsetenv(name.c_str()) != 0)
            {
                error_code = ft_global_error_stack_drop_last_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_INVALID_ARGUMENT;
                error_code = su_environment_unlock_with_error(error_code);
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_drop_last_error();
        }
        index += 1;
    }
    size_t snapshot_count = snapshot->entries.size();
    if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = su_environment_unlock_with_error(snapshot->entries.get_error());
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    index = 0;
    while (index < snapshot_count)
    {
        const ft_string &entry = snapshot->entries[index];
        ft_string        name;
        ft_string        value;
        int              has_value;

        error_code = su_environment_split_entry(entry.c_str(), name, value, &has_value);
        if (error_code != FT_ERR_SUCCESSS)
        {
            error_code = su_environment_unlock_with_error(error_code);
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        if (has_value != 0)
        {
            if (ft_setenv(name.c_str(), value.c_str(), 1) != 0)
            {
                error_code = ft_global_error_stack_drop_last_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_INVALID_ARGUMENT;
                error_code = su_environment_unlock_with_error(error_code);
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_drop_last_error();
        }
        else
        {
            if (ft_unsetenv(name.c_str()) != 0)
            {
                error_code = ft_global_error_stack_drop_last_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_INVALID_ARGUMENT;
                error_code = su_environment_unlock_with_error(error_code);
                ft_global_error_stack_push(error_code);
                return (-1);
            }
            ft_global_error_stack_drop_last_error();
        }
        index += 1;
    }
    mutex_error = su_environment_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void su_environment_snapshot_dispose(t_su_environment_snapshot *snapshot)
{
    if (snapshot == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    snapshot->entries.clear();
    if (snapshot->entries.get_error() != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(snapshot->entries.get_error());
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int su_environment_sandbox_begin(t_su_environment_snapshot *snapshot)
{
    int capture_result;
    int error_code;

    capture_result = su_environment_snapshot_capture(snapshot);
    error_code = ft_global_error_stack_drop_last_error();
    if (capture_result != 0)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_environment_sandbox_end(t_su_environment_snapshot *snapshot)
{
    int restore_result;
    int restore_error;
    int dispose_error;

    restore_result = su_environment_snapshot_restore(snapshot);
    restore_error = ft_global_error_stack_drop_last_error();
    su_environment_snapshot_dispose(snapshot);
    dispose_error = ft_global_error_stack_drop_last_error();
    if (restore_result != 0)
    {
        if (restore_error == FT_ERR_SUCCESSS)
            restore_error = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(restore_error);
        return (-1);
    }
    if (dispose_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(dispose_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
