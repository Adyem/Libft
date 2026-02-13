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

static int su_health_copy_entry(const s_su_health_check_entry &source,
    s_su_health_check_entry &destination)
{
    destination.name = source.name;
    destination.callback = source.callback;
    destination.context = source.context;
    return (FT_ERR_SUCCESS);
}

static int su_health_execute_entry(const s_su_health_check_entry &entry,
    t_su_health_check_result *result)
{
    ft_string    detail;
    bool         healthy;
    int          callback_result;
    int          error_code;

    healthy = true;
    error_code = FT_ERR_SUCCESS;
    if (entry.callback == ft_nullptr)
    {
        healthy = false;
        error_code = FT_ERR_INVALID_POINTER;
    }
    else
    {
        callback_result = entry.callback(entry.context, detail);
        if (callback_result != 0)
        {
            healthy = false;
            error_code = FT_ERR_INTERNAL;
        }
    }
    if (result != ft_nullptr)
    {
        result->name = entry.name;
        result->healthy = healthy;
        result->detail = detail;
        result->error_code = error_code;
    }
    if (healthy == false)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int su_health_register_check(const char *name, t_su_health_check check, void *context)
{
    size_t                   index;
    s_su_health_check_entry  stored_entry;

    if (name == ft_nullptr || name[0] == '\0')
        return (-1);
    if (check == ft_nullptr)
        return (-1);
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index].name == name)
            return (-1);
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
        return (-1);
    }
    return (0);
}

int su_health_unregister_check(const char *name)
{
    size_t index;

    if (name == ft_nullptr)
        return (-1);
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
    index = 0;
    while (index < g_su_health_checks.size())
    {
        if (g_su_health_checks[index].name == name)
        {
            g_su_health_checks.erase(g_su_health_checks.begin() + index);
            return (0);
        }
        index += 1;
    }
    return (-1);
}

void su_health_clear_checks(void)
{
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);

    g_su_health_checks.clear();
    return ;
}

int su_health_run_checks(t_su_health_check_result *results, size_t capacity, size_t *count)
{
    std::vector<s_su_health_check_entry> local_checks;
    size_t                               index;
    int                                  execution_result;

    if (count == ft_nullptr)
        return (-1);
    {
        std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
        try
        {
            local_checks.reserve(g_su_health_checks.size());
        }
        catch (const std::bad_alloc &)
        {
            return (-1);
        }
        index = 0;
        while (index < g_su_health_checks.size())
        {
            s_su_health_check_entry copy_entry;

            if (su_health_copy_entry(g_su_health_checks[index], copy_entry) != FT_ERR_SUCCESS)
                return (-1);
            try
            {
                local_checks.push_back(copy_entry);
            }
            catch (const std::bad_alloc &)
            {
                return (-1);
            }
            index += 1;
        }
    }
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

int su_health_run_check(const char *name, t_su_health_check_result *result)
{
    s_su_health_check_entry found_entry;
    size_t                  index;
    bool                    found;

    if (name == ft_nullptr)
        return (-1);
    found = false;
    {
        std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
        index = 0;
        while (index < g_su_health_checks.size())
        {
            if (g_su_health_checks[index].name == name)
            {
                if (su_health_copy_entry(g_su_health_checks[index], found_entry) != FT_ERR_SUCCESS)
                    return (-1);
                found = true;
                break ;
            }
            index += 1;
        }
    }
    if (found == false)
        return (-1);
    if (su_health_execute_entry(found_entry, result) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
