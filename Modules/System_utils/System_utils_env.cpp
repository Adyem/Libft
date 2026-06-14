#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include <cerrno>
#include <cstdlib>
#include <utility>
#include "../Basic/limits.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/vector.hpp"

static pthread_mutex_t *g_env_mutex = ft_nullptr;

static int32_t su_environment_lock_mutex(void)
{
    int32_t pthread_error;

    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    pthread_error = pthread_mutex_lock(g_env_mutex);
    if (pthread_error != 0)
        return (cmp_map_system_error_to_ft(pthread_error));
    return (FT_ERR_SUCCESS);
}

static int32_t su_environment_unlock_mutex(void)
{
    int32_t pthread_error;

    if (g_env_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    pthread_error = pthread_mutex_unlock(g_env_mutex);
    if (pthread_error != 0)
        return (cmp_map_system_error_to_ft(pthread_error));
    return (FT_ERR_SUCCESS);
}

int32_t su_environment_enable_thread_safety(void)
{
    pthread_mutex_t  *mutex_pointer;
    int32_t             pthread_error;

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

static int32_t g_environment_thread_safety_initializer = su_environment_enable_thread_safety();

static char **su_environment_entries(void)
{
    return (cmp_get_environ_entries());
}

static int32_t su_environment_split_entry(
    const char *entry,
    ft_string &name,
    ft_string &value,
    int32_t *has_value)
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
    int32_t     lock_error;
    int32_t     unlock_error;
    char        *result;

    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (name == ft_nullptr || *name == '\0')
    {
        (void)su_environment_unlock_mutex();
        return (ft_nullptr);
    }
    result = getenv(name);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    return (result);
}

int32_t su_setenv(const char *name, const char *value, int32_t overwrite)
{
    int32_t lock_error;
    int32_t unlock_error;
    int32_t result;

    if (name == ft_nullptr || value == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (*name == '\0' || ft_strchr(name, '=') != ft_nullptr)
        return (FT_ERR_INVALID_OPERATION);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = cmp_setenv(name, value, overwrite);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int32_t su_unsetenv(const char *name)
{
    int32_t lock_error;
    int32_t unlock_error;
    int32_t result;

    if (name == ft_nullptr || *name == '\0' || ft_strchr(name, '=') != ft_nullptr)
        return (-1);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = cmp_unsetenv(name);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    if (result == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    if (result > FT_ERR_SUCCESS)
        return (result);
#if defined(_WIN32) || defined(_WIN64)
    if (GetLastError() != 0 || WSAGetLastError() != 0)
        return (-1);
#endif
    if (errno == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int32_t su_putenv(char *string)
{
    int32_t result;
    int32_t lock_error;
    int32_t unlock_error;

    if (string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = cmp_putenv(string);
    unlock_error = su_environment_unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int32_t su_environment_snapshot_capture(t_su_environment_snapshot *snapshot)
{
    int32_t     lock_error;
    int32_t     unlock_error;
    char    **entries;
    ft_size_t   index;
    int32_t     string_error;

    if (snapshot == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (snapshot->entries.is_initialised() != FT_CLASS_STATE_INITIALISED)
    {
        string_error = snapshot->entries.initialize();
        if (string_error != FT_ERR_SUCCESS)
            return (string_error);
    }
    lock_error = su_environment_lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    snapshot->entries.clear();
    if (snapshot->entries.get_error() != FT_ERR_SUCCESS)
    {
        (void)su_environment_unlock_mutex();
        return (snapshot->entries.get_error());
    }
    entries = su_environment_entries();
    if (entries != ft_nullptr)
    {
        index = 0;
        while (entries[index] != ft_nullptr)
        {
            ft_string entry_copy;
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

int32_t su_environment_snapshot_restore(const t_su_environment_snapshot *snapshot)
{
    int32_t         lock_error;
    int32_t         unlock_error;
    char        **entries;
    ft_string   name;
    ft_string   value;
    int32_t         has_value;
    int32_t         error_code;
    ft_size_t   index;

    if (snapshot == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = name.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = value.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
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
        if (cmp_unsetenv(name.c_str()) != 0)
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
            if (cmp_unsetenv(name.c_str()) != 0)
            {
                error_code = cmp_map_system_error_to_ft(errno);
                (void)su_environment_unlock_mutex();
                return (error_code);
            }
        }
        else
        {
            if (cmp_setenv(name.c_str(), value.c_str(), 1) != 0)
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
    if (snapshot == ft_nullptr)
        return ;
    if (snapshot->entries.is_initialised() == FT_CLASS_STATE_INITIALISED)
        snapshot->entries.clear();
    return ;
}

int32_t su_environment_sandbox_begin(t_su_environment_snapshot *snapshot)
{
    return (su_environment_snapshot_capture(snapshot));
}

int32_t su_environment_sandbox_end(t_su_environment_snapshot *snapshot)
{
    int32_t restore_error;

    restore_error = su_environment_snapshot_restore(snapshot);
    su_environment_snapshot_dispose(snapshot);
    return (restore_error);
}
