#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
#endif
#include <cerrno>
#include <cstdlib>
#include <utility>

static pthread_mutex_t *g_env_mutex = ft_nullptr;

static int su_environment_lock_mutex(void)
{
    int pthread_error;

    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    pthread_error = pthread_mutex_lock(g_env_mutex);
    if (pthread_error != 0)
        return (cmp_map_system_error_to_ft(pthread_error));
    return (FT_ERR_SUCCESS);
}

static int su_environment_unlock_mutex(void)
{
    int pthread_error;

    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    pthread_error = pthread_mutex_unlock(g_env_mutex);
    if (pthread_error != 0)
        return (cmp_map_system_error_to_ft(pthread_error));
    return (FT_ERR_SUCCESS);
}

int su_environment_enable_thread_safety(void)
{
    pthread_mutex_t  *mutex_pointer;
    int             pthread_error;

    if (g_env_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = static_cast<pthread_mutex_t *>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    pthread_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
    if (pthread_error != 0)
    {
        std::free(mutex_pointer);
        return (cmp_map_system_error_to_ft(pthread_error));
    }
    g_env_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void su_environment_disable_thread_safety(void)
{
    if (g_env_mutex != ft_nullptr)
    {
        pthread_mutex_destroy(g_env_mutex);
        std::free(g_env_mutex);
        g_env_mutex = ft_nullptr;
    }
    return ;
}

static int g_environment_thread_safety_initializer = su_environment_enable_thread_safety();

static char **su_environment_entries(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (_environ);
#else
    return (environ);
#endif
}

static int su_environment_split_entry(
    const char *entry,
    ft_string &name,
    ft_string &value,
    int *has_value)
{
    const char  *equals_location;
    ft_size_t   name_length;
    int32_t     string_error;

    if (entry == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    equals_location = ft_strchr(entry, '=');
    if (equals_location == ft_nullptr)
    {
        string_error = name.assign(entry, ft_strlen_size_t(entry));
        if (string_error != FT_ERR_SUCCESS)
            return (string_error);
        string_error = value.clear();
        if (string_error != FT_ERR_SUCCESS)
            return (string_error);
        if (has_value != ft_nullptr)
            *has_value = 0;
        return (FT_ERR_SUCCESS);
    }
    name_length = static_cast<ft_size_t>(equals_location - entry);
    string_error = name.assign(entry, name_length);
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    string_error = value.assign(
            equals_location + 1,
            ft_strlen_size_t(equals_location + 1));
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    if (has_value != ft_nullptr)
        *has_value = 1;
    return (FT_ERR_SUCCESS);
}

char *su_getenv(const char *name)
{
    char    *result;
    int     lock_error;
    int     unlock_error;

    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    result = ft_getenv(name);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    return (result);
}

int su_setenv(const char *name, const char *value, int overwrite)
{
    int result;
    int lock_error;
    int unlock_error;

    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = ft_setenv(name, value, overwrite);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int su_putenv(char *string)
{
    int result;
    int lock_error;
    int unlock_error;

    if (string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = putenv(string);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int su_environment_snapshot_capture(t_su_environment_snapshot *snapshot)
{
    int     lock_error;
    int     unlock_error;
    char    **entries;
    ft_size_t   index;
    ft_string   entry_copy;
    int32_t     string_error;

    if (snapshot == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    snapshot->entries.clear();
    entries = su_environment_entries();
    if (entries != ft_nullptr)
    {
        index = 0;
        while (entries[index] != ft_nullptr)
        {
            string_error = entry_copy.initialize(entries[index]);
            if (string_error != FT_ERR_SUCCESS)
            {
                (void)su_environment_unlock_mutex();
                return (string_error);
            }
            snapshot->entries.push_back(ft_move(entry_copy));
            index = index + 1;
        }
    }
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int su_environment_snapshot_restore(const t_su_environment_snapshot *snapshot)
{
    int         lock_error;
    int         unlock_error;
    char        **entries;
    ft_string   name;
    ft_string   value;
    int         has_value;
    int         error_code;
    ft_size_t   index;

    if (snapshot == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    entries = su_environment_entries();
    while (entries != ft_nullptr && entries[0] != ft_nullptr)
    {
        error_code = su_environment_split_entry(entries[0], name, value, &has_value);
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)su_environment_unlock_mutex();
            return (error_code);
        }
        if (ft_unsetenv(name.c_str()) != 0)
        {
            error_code = cmp_map_system_error_to_ft(errno);
            (void)su_environment_unlock_mutex();
            return (error_code);
        }
        entries = su_environment_entries();
    }
    index = 0;
    while (index < snapshot->entries.size())
    {
        error_code = su_environment_split_entry(
                snapshot->entries[index].c_str(),
                name,
                value,
                &has_value);
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)su_environment_unlock_mutex();
            return (error_code);
        }
        if (has_value == 0)
        {
            if (ft_unsetenv(name.c_str()) != 0)
            {
                error_code = cmp_map_system_error_to_ft(errno);
                (void)su_environment_unlock_mutex();
                return (error_code);
            }
        }
        else
        {
            if (ft_setenv(name.c_str(), value.c_str(), 1) != 0)
            {
                error_code = cmp_map_system_error_to_ft(errno);
                (void)su_environment_unlock_mutex();
                return (error_code);
            }
        }
        index = index + 1;
    }
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

void su_environment_snapshot_dispose(t_su_environment_snapshot *snapshot)
{
    if (snapshot != ft_nullptr)
        snapshot->entries.clear();
    return ;
}

int su_environment_sandbox_begin(t_su_environment_snapshot *snapshot)
{
    return (su_environment_snapshot_capture(snapshot));
}

int su_environment_sandbox_end(t_su_environment_snapshot *snapshot)
{
    int restore_error;

    restore_error = su_environment_snapshot_restore(snapshot);
    su_environment_snapshot_dispose(snapshot);
    return (restore_error);
}
