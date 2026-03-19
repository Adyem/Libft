#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <mutex>
#include <new>
#include <vector>

typedef struct s_su_health_check_entry
{
    ft_string           name;
    t_su_health_check   callback;
    void                *context;
}   t_su_health_check_entry;

static std::vector<s_su_health_check_entry> g_su_health_checks;
static std::mutex                           g_su_health_checks_mutex;

static int32_t su_health_copy_entry(const s_su_health_check_entry &source,
    s_su_health_check_entry &destination)
{
    destination.name = source.name;
    destination.callback = source.callback;
    destination.context = source.context;
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
    s_su_health_check_entry  stored_entry;
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
        if (g_su_health_checks[index].name == name)
        {
            if (lock_acquired)
                g_su_health_checks_mutex.unlock();
            return (-1);
        }
        index += 1;
    }
    stored_entry.callback = check;
    stored_entry.context = context;
    stored_entry.name = name;
    try
    {
        g_su_health_checks.push_back(stored_entry);
    }
    catch (const std::bad_alloc &)
    {
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
        if (g_su_health_checks[index].name == name)
        {
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
    g_su_health_checks_mutex.lock();
    g_su_health_checks.clear();
    g_su_health_checks_mutex.unlock();
    return ;
}

int32_t su_health_run_checks(t_su_health_check_result *results,
    ft_size_t capacity, ft_size_t *count)
{
    std::vector<s_su_health_check_entry> local_checks;
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
        s_su_health_check_entry copy_entry;

        if (su_health_copy_entry(g_su_health_checks[index], copy_entry) != FT_ERR_SUCCESS)
        {
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
        return (-1);
    index = 0;
    while (index < local_checks.size())
    {
        t_su_health_check_result *result_slot;

        result_slot = ft_nullptr;
        if (results != ft_nullptr)
            result_slot = &results[index];
        execution_result = su_health_execute_entry(local_checks[index], result_slot);
        if (execution_result != FT_ERR_SUCCESS)
            return (-1);
        index += 1;
    }
    return (0);
}

int32_t su_health_run_check(const char *name, t_su_health_check_result *result)
{
    s_su_health_check_entry found_entry;
    ft_size_t               index;
    ft_bool                    found;
    ft_bool                    lock_acquired;

    if (name == ft_nullptr)
        return (-1);
    found = FT_FALSE;
    lock_acquired = FT_FALSE;
    g_su_health_checks_mutex.lock();
    lock_acquired = FT_TRUE;
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index].name == name)
        {
            if (su_health_copy_entry(g_su_health_checks[index], found_entry) != FT_ERR_SUCCESS)
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
    if (su_health_execute_entry(found_entry, result) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
