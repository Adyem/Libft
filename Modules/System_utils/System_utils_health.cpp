#include "system_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <mutex>
#include <new>
#include <vector>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

typedef struct s_su_health_check_entry
{
    ft_string           name;
    t_su_health_check   callback;
    void                *context;
}   t_su_health_check_entry;

static std::vector<s_su_health_check_entry *> g_su_health_checks;
static std::mutex                           g_su_health_checks_mutex;

static void su_health_destroy_entry(s_su_health_check_entry *entry)
{
    if (entry == ft_nullptr)
        return ;
    (void)entry->name.destroy();
    delete entry;
    return ;
}

static int32_t su_health_copy_entry(const s_su_health_check_entry *source,
    s_su_health_check_entry **destination)
{
    s_su_health_check_entry *new_entry;

    if (source == ft_nullptr || destination == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    new_entry = new (std::nothrow) s_su_health_check_entry;
    if (new_entry == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    new_entry->callback = source->callback;
    new_entry->context = source->context;
    if (new_entry->name.initialize(source->name) != FT_ERR_SUCCESS)
    {
        delete new_entry;
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    *destination = new_entry;
    return (FT_ERR_SUCCESS);
}

static int32_t su_health_execute_entry(const s_su_health_check_entry &entry,
    t_su_health_check_result *result)
{
    ft_string    detail;
    ft_bool         healthy;
    int32_t          callback_result;
    int32_t          error_code;

    healthy = FT_TRUE;
    error_code = FT_ERR_SUCCESS;
    if (detail.initialize() != FT_ERR_SUCCESS)
    {
        healthy = FT_FALSE;
        error_code = FT_ERR_NO_MEMORY;
    }
    if (entry.callback == ft_nullptr)
    {
        healthy = FT_FALSE;
        error_code = FT_ERR_INVALID_POINTER;
    }
    else if (error_code == FT_ERR_SUCCESS)
    {
        callback_result = entry.callback(entry.context, detail);
        if (callback_result != 0)
        {
            healthy = FT_FALSE;
            error_code = FT_ERR_INTERNAL;
        }
    }
    if (result != ft_nullptr)
    {
        if (result->name.is_initialised() != FT_CLASS_STATE_INITIALISED)
            (void)result->name.initialize();
        if (result->detail.is_initialised() != FT_CLASS_STATE_INITIALISED)
            (void)result->detail.initialize();
        result->name = entry.name;
        result->healthy = healthy;
        result->detail = detail;
        result->error_code = error_code;
    }
    if (healthy == FT_FALSE)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int32_t su_health_register_check(const char *name, t_su_health_check check, void *context)
{
    ft_size_t                index;
    s_su_health_check_entry  *stored_entry;
    ft_bool                     lock_acquired;

    if (name == ft_nullptr || name[0] == '\0')
        return (-1);
    if (check == ft_nullptr)
        return (-1);
    lock_acquired = FT_FALSE;
    g_su_health_checks_mutex.lock();
    lock_acquired = FT_TRUE;
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index]->name == name)
        {
            if (lock_acquired)
                g_su_health_checks_mutex.unlock();
            return (-1);
        }
        index += 1;
    }
    stored_entry = new (std::nothrow) s_su_health_check_entry;
    if (stored_entry == ft_nullptr)
    {
        if (lock_acquired)
            g_su_health_checks_mutex.unlock();
        return (-1);
    }
    stored_entry->callback = check;
    stored_entry->context = context;
    if (stored_entry->name.initialize(name) != FT_ERR_SUCCESS)
    {
        delete stored_entry;
        if (lock_acquired)
            g_su_health_checks_mutex.unlock();
        return (-1);
    }
    try
    {
        g_su_health_checks.push_back(stored_entry);
    }
    catch (const std::bad_alloc &)
    {
        su_health_destroy_entry(stored_entry);
        if (lock_acquired)
            g_su_health_checks_mutex.unlock();
        return (-1);
    }
    if (lock_acquired)
        g_su_health_checks_mutex.unlock();
    return (0);
}

int32_t su_health_unregister_check(const char *name)
{
    ft_size_t index;
    ft_bool   lock_acquired;

    if (name == ft_nullptr)
        return (-1);
    lock_acquired = FT_FALSE;
    g_su_health_checks_mutex.lock();
    lock_acquired = FT_TRUE;
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index]->name == name)
        {
            su_health_destroy_entry(g_su_health_checks[index]);
            g_su_health_checks.erase(g_su_health_checks.begin() + index);
            if (lock_acquired)
                g_su_health_checks_mutex.unlock();
            return (0);
        }
        index += 1;
    }
    if (lock_acquired)
        g_su_health_checks_mutex.unlock();
    return (-1);
}

void su_health_clear_checks(void)
{
    ft_size_t index;

    g_su_health_checks_mutex.lock();
    index = 0;
    while (index < g_su_health_checks.size())
    {
        su_health_destroy_entry(g_su_health_checks[index]);
        index += 1;
    }
    std::vector<s_su_health_check_entry *>().swap(g_su_health_checks);
    g_su_health_checks_mutex.unlock();
    return ;
}

static void su_health_clear_local_checks(std::vector<s_su_health_check_entry *> &checks)
{
    ft_size_t index;

    index = 0;
    while (index < checks.size())
    {
        su_health_destroy_entry(checks[index]);
        index += 1;
    }
    checks.clear();
    return ;
}

int32_t su_health_run_checks(t_su_health_check_result *results,
    ft_size_t capacity, ft_size_t *count)
{
    std::vector<s_su_health_check_entry *> local_checks;
    ft_size_t                            index;
    int32_t                                  execution_result;
    ft_bool                                 lock_acquired;

    if (count == ft_nullptr)
        return (-1);
    lock_acquired = FT_FALSE;
    g_su_health_checks_mutex.lock();
    lock_acquired = FT_TRUE;
    try
    {
        local_checks.reserve(g_su_health_checks.size());
    }
    catch (const std::bad_alloc &)
    {
        if (lock_acquired)
            g_su_health_checks_mutex.unlock();
        return (-1);
    }
    index = 0;
    while (index < g_su_health_checks.size())
    {
        s_su_health_check_entry *copy_entry;

        copy_entry = ft_nullptr;
        if (su_health_copy_entry(g_su_health_checks[index], &copy_entry) != FT_ERR_SUCCESS)
        {
            su_health_clear_local_checks(local_checks);
            if (lock_acquired)
                g_su_health_checks_mutex.unlock();
            return (-1);
        }
        try
        {
            local_checks.push_back(copy_entry);
        }
        catch (const std::bad_alloc &)
        {
            su_health_destroy_entry(copy_entry);
            su_health_clear_local_checks(local_checks);
            if (lock_acquired)
                g_su_health_checks_mutex.unlock();
            return (-1);
        }
        index += 1;
    }
    if (lock_acquired)
        g_su_health_checks_mutex.unlock();
    *count = local_checks.size();
    if (results != ft_nullptr && capacity < local_checks.size())
    {
        su_health_clear_local_checks(local_checks);
        return (-1);
    }
    index = 0;
    while (index < local_checks.size())
    {
        t_su_health_check_result *result_slot;

        result_slot = ft_nullptr;
        if (results != ft_nullptr)
            result_slot = &results[index];
        execution_result = su_health_execute_entry(*local_checks[index], result_slot);
        if (execution_result != FT_ERR_SUCCESS)
        {
            su_health_clear_local_checks(local_checks);
            return (-1);
        }
        index += 1;
    }
    su_health_clear_local_checks(local_checks);
    return (0);
}

int32_t su_health_run_check(const char *name, t_su_health_check_result *result)
{
    s_su_health_check_entry *found_entry;
    ft_size_t               index;
    ft_bool                    found;
    ft_bool                    lock_acquired;

    if (name == ft_nullptr)
        return (-1);
    found_entry = ft_nullptr;
    found = FT_FALSE;
    lock_acquired = FT_FALSE;
    g_su_health_checks_mutex.lock();
    lock_acquired = FT_TRUE;
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index]->name == name)
        {
            if (su_health_copy_entry(g_su_health_checks[index], &found_entry) != FT_ERR_SUCCESS)
            {
                if (lock_acquired)
                    g_su_health_checks_mutex.unlock();
                return (-1);
            }
            found = FT_TRUE;
            break ;
        }
        index += 1;
    }
    if (lock_acquired)
        g_su_health_checks_mutex.unlock();
    if (found == FT_FALSE)
        return (-1);
    if (su_health_execute_entry(*found_entry, result) != FT_ERR_SUCCESS)
    {
        su_health_destroy_entry(found_entry);
        return (-1);
    }
    su_health_destroy_entry(found_entry);
    return (0);
}
