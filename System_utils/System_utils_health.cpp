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
static std::mutex                          g_su_health_checks_mutex;

static int  su_health_copy_entry(const s_su_health_check_entry &source, s_su_health_check_entry &destination)
{
    destination.name = source.name;
    if (destination.name.get_error() != FT_ERR_SUCCESSS)
        return (destination.name.get_error());
    destination.callback = source.callback;
    destination.context = source.context;
    return (FT_ERR_SUCCESSS);
}

static int  su_health_execute_entry(const s_su_health_check_entry &entry, t_su_health_check_result *result)
{
    ft_string   detail;
    bool        healthy;
    int         error_code;

    healthy = true;
    error_code = FT_ERR_SUCCESSS;
    if (entry.callback == ft_nullptr)
    {
        healthy = false;
        error_code = FT_ERR_INVALID_POINTER;
    }
    else
    {
        int callback_result;
        int detail_error;

        callback_result = entry.callback(entry.context, detail);
        detail_error = detail.get_error();
        if (detail_error != FT_ERR_SUCCESSS)
        {
            healthy = false;
            error_code = detail_error;
        }
        else if (callback_result != 0)
        {
            healthy = false;
            error_code = FT_ERR_INTERNAL;
        }
    }
    if (result)
    {
        result->name = entry.name;
        if (result->name.get_error() != FT_ERR_SUCCESSS)
        {
            return (result->name.get_error());
        }
        result->healthy = healthy;
        result->detail = detail;
        if (result->detail.get_error() != FT_ERR_SUCCESSS)
        {
            return (result->detail.get_error());
        }
        result->error_code = error_code;
    }
    if (!healthy)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        return (error_code);
    }
    return (FT_ERR_SUCCESSS);
}

int su_health_register_check(const char *name, t_su_health_check check, void *context)
{
    int error_code;

    if (!name || name[0] == '\0')
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (!check)
    {
        error_code = FT_ERR_INVALID_POINTER;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
    size_t index = 0;
    size_t count = g_su_health_checks.size();
    while (index < count)
    {
        s_su_health_check_entry &existing = g_su_health_checks[index];

        if (existing.name == name)
        {
            error_code = FT_ERR_ALREADY_EXISTS;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
        index += 1;
    }
    s_su_health_check_entry stored_entry;

    stored_entry.callback = check;
    stored_entry.context = context;
    stored_entry.name = name;
    if (stored_entry.name.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = stored_entry.name.get_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    try
    {
        g_su_health_checks.push_back(stored_entry);
    }
    catch (const std::bad_alloc &)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_health_unregister_check(const char *name)
{
    int error_code;

    if (!name)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
    size_t index = 0;
    size_t count = g_su_health_checks.size();
    while (index < count)
    {
        s_su_health_check_entry &existing = g_su_health_checks[index];

        if (existing.name == name)
        {
            g_su_health_checks.erase(g_su_health_checks.begin() + index);
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (0);
        }
        index += 1;
    }
    error_code = FT_ERR_NOT_FOUND;
    ft_global_error_stack_push(error_code);
    return (-1);
}

void    su_health_clear_checks(void)
{
    std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);

    g_su_health_checks.clear();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int su_health_run_checks(t_su_health_check_result *results, size_t capacity, size_t *count)
{
    int     final_result;
    int     final_error;
    size_t  produced_count;

    final_result = 0;
    final_error = FT_ERR_SUCCESSS;
    produced_count = 0;
    {
        std::vector<s_su_health_check_entry> local_checks;
        bool                                 processing;

        processing = true;
        {
            std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
            size_t total = g_su_health_checks.size();
            try
            {
                local_checks.reserve(total);
            }
            catch (const std::bad_alloc &)
            {
                final_result = -1;
                final_error = FT_ERR_NO_MEMORY;
                processing = false;
            }
            size_t index = 0;
            while (processing && index < total)
            {
                s_su_health_check_entry copy;

                int copy_error;

                copy_error = su_health_copy_entry(g_su_health_checks[index], copy);
                if (copy_error != FT_ERR_SUCCESSS)
                {
                    final_result = -1;
                    final_error = copy_error;
                    processing = false;
                }
                else
                {
                    try
                    {
                        local_checks.push_back(copy);
                    }
                    catch (const std::bad_alloc &)
                    {
                        final_result = -1;
                        final_error = FT_ERR_NO_MEMORY;
                        processing = false;
                    }
                }
                index += 1;
            }
        }
        if (processing)
        {
            produced_count = local_checks.size();
            if (!count)
            {
                final_result = -1;
                final_error = FT_ERR_INVALID_ARGUMENT;
                processing = false;
            }
            else if (results && capacity < produced_count)
            {
                final_result = -1;
                final_error = FT_ERR_OUT_OF_RANGE;
                processing = false;
            }
        }
        if (processing)
        {
            size_t  index;
            bool    has_failure;
            int     first_error;

            index = 0;
            has_failure = false;
            first_error = FT_ERR_SUCCESSS;
            while (index < local_checks.size())
            {
                t_su_health_check_result *slot;
                int execution_result;

                slot = ft_nullptr;
                if (results && index < capacity)
                    slot = &results[index];
                execution_result = su_health_execute_entry(local_checks[index], slot);
                if (execution_result != FT_ERR_SUCCESSS)
                {
                    if (!has_failure)
                    {
                        has_failure = true;
                        int execution_error;

                        execution_error = execution_result;
                        if (execution_error == FT_ERR_SUCCESSS && slot != ft_nullptr)
                            execution_error = slot->error_code;
                        if (execution_error == FT_ERR_SUCCESSS)
                            execution_error = FT_ERR_INTERNAL;
                        first_error = execution_error;
                    }
                }
                index += 1;
            }
            if (has_failure)
            {
                final_result = -1;
                final_error = first_error;
            }
        }
    }
    if (count)
        *count = produced_count;
    if (final_result != 0)
    {
        if (final_error == FT_ERR_SUCCESSS)
            final_error = FT_ERR_INTERNAL;
        ft_global_error_stack_push(final_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_health_run_check(const char *name, t_su_health_check_result *result)
{
    int final_result;
    int final_error;

    if (!name)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    final_result = 0;
    final_error = FT_ERR_SUCCESSS;
    bool found;
    s_su_health_check_entry found_entry;

    found = false;
    {
        std::lock_guard<std::mutex> guard(g_su_health_checks_mutex);
        size_t index;
        size_t total;

        index = 0;
        total = g_su_health_checks.size();
        while (index < total)
        {
            s_su_health_check_entry &candidate = g_su_health_checks[index];

            if (candidate.name == name)
            {
                int copy_error;

                copy_error = su_health_copy_entry(candidate, found_entry);
                if (copy_error != FT_ERR_SUCCESSS)
                {
                    final_result = -1;
                    final_error = copy_error;
                }
                else
                    found = true;
                break ;
            }
            index += 1;
        }
    }
    if (final_result == 0 && !found)
    {
        final_result = -1;
        final_error = FT_ERR_NOT_FOUND;
    }
    if (final_result == 0 && found)
    {
        int execution_result;

        execution_result = su_health_execute_entry(found_entry, result);
        if (execution_result != FT_ERR_SUCCESSS)
        {
            final_result = -1;
            if (result != ft_nullptr)
                final_error = result->error_code;
            else
            {
                final_error = execution_result;
                if (final_error == FT_ERR_SUCCESSS)
                    final_error = FT_ERR_INTERNAL;
            }
        }
    }
    if (final_result != 0)
    {
        ft_global_error_stack_push(final_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
