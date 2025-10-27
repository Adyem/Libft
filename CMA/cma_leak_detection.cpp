#include "CMA.hpp"
#include "cma_internal.hpp"
#include "cma_guard_vector.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

struct s_cma_leak_record
{
    void    *pointer;
    ft_size_t    size;
};

static thread_local bool g_cma_leak_detection_enabled = false;
static thread_local bool g_cma_leak_detection_error = false;
static thread_local cma_guard_vector<s_cma_leak_record> g_cma_leak_records;
static thread_local ft_size_t g_cma_leak_outstanding_bytes = 0;

static bool cma_leak_tracker_is_active()
{
    if (!g_cma_leak_detection_enabled)
        return (false);
    if (g_cma_leak_detection_error)
        return (false);
    return (true);
}

static void cma_leak_tracker_handle_error(int error_code)
{
    g_cma_leak_detection_enabled = false;
    g_cma_leak_detection_error = true;
    ft_errno = error_code;
    return ;
}

static bool cma_leak_tracker_clear_records(bool reset_error_flag)
{
    g_cma_leak_records.clear();
    if (g_cma_leak_records.get_error() != ER_SUCCESS)
    {
        cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
        return (false);
    }
    g_cma_leak_outstanding_bytes = 0;
    if (reset_error_flag)
        g_cma_leak_detection_error = false;
    return (true);
}

static bool cma_leak_report_append(ft_string &target, const char *text)
{
    target.append(text);
    if (target.get_error() != ER_SUCCESS)
        return (false);
    return (true);
}

static bool cma_leak_report_append_string(ft_string &target, const ft_string &value)
{
    if (value.get_error() != ER_SUCCESS)
        return (false);
    target.append(value.c_str());
    if (target.get_error() != ER_SUCCESS)
        return (false);
    return (true);
}

static void cma_leak_tracker_resume(bool was_enabled, bool error_state)
{
    if (was_enabled && !error_state)
        g_cma_leak_detection_enabled = true;
    else
        g_cma_leak_detection_enabled = false;
    g_cma_leak_detection_error = error_state;
    return ;
}

void cma_leak_tracker_record_allocation(void *memory_pointer, ft_size_t size)
{
    s_cma_leak_record record;
    int entry_errno;

    if (!cma_leak_tracker_is_active())
        return ;
    if (memory_pointer == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    record.pointer = memory_pointer;
    record.size = size;
    g_cma_leak_records.push_back(record);
    if (g_cma_leak_records.get_error() != ER_SUCCESS)
    {
        cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
        return ;
    }
    g_cma_leak_outstanding_bytes += size;
    ft_errno = entry_errno;
    return ;
}

void cma_leak_tracker_record_free(void *memory_pointer)
{
    int entry_errno;
    size_t record_index;
    size_t record_count;

    if (!cma_leak_tracker_is_active())
        return ;
    if (memory_pointer == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    record_count = g_cma_leak_records.size();
    record_index = 0;
    while (record_index < record_count)
    {
        s_cma_leak_record record;
        size_t last_index;
        s_cma_leak_record last_record;

        record = g_cma_leak_records[record_index];
        if (g_cma_leak_records.get_error() != ER_SUCCESS)
        {
            cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
            return ;
        }
        if (record.pointer == memory_pointer)
        {
            if (g_cma_leak_outstanding_bytes >= record.size)
                g_cma_leak_outstanding_bytes -= record.size;
            else
                g_cma_leak_outstanding_bytes = 0;
            last_index = g_cma_leak_records.size();
            if (last_index == 0)
            {
                ft_errno = entry_errno;
                return ;
            }
            last_index -= 1;
            if (record_index != last_index)
            {
                last_record = g_cma_leak_records[last_index];
                if (g_cma_leak_records.get_error() != ER_SUCCESS)
                {
                    cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
                    return ;
                }
                g_cma_leak_records[record_index] = last_record;
                if (g_cma_leak_records.get_error() != ER_SUCCESS)
                {
                    cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
                    return ;
                }
            }
            g_cma_leak_records.pop_back();
            if (g_cma_leak_records.get_error() != ER_SUCCESS)
            {
                cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
                return ;
            }
            ft_errno = entry_errno;
            return ;
        }
        record_index += 1;
    }
    ft_errno = entry_errno;
    return ;
}

void cma_leak_detection_enable(void)
{
    g_cma_leak_detection_enabled = true;
    g_cma_leak_detection_error = false;
    ft_errno = ER_SUCCESS;
    return ;
}

void cma_leak_detection_disable(void)
{
    g_cma_leak_detection_enabled = false;
    ft_errno = ER_SUCCESS;
    return ;
}

void cma_leak_detection_clear(void)
{
    int entry_errno;

    entry_errno = ft_errno;
    if (!cma_leak_tracker_clear_records(true))
        return ;
    ft_errno = entry_errno;
    return ;
}

bool cma_leak_detection_is_enabled(void)
{
    if (!g_cma_leak_detection_enabled)
        return (false);
    if (g_cma_leak_detection_error)
        return (false);
    return (true);
}

ft_size_t cma_leak_detection_outstanding_allocations(void)
{
    ft_size_t allocation_count;
    int entry_errno;

    entry_errno = ft_errno;
    allocation_count = static_cast<ft_size_t>(g_cma_leak_records.size());
    ft_errno = entry_errno;
    return (allocation_count);
}

ft_size_t cma_leak_detection_outstanding_bytes(void)
{
    ft_size_t outstanding_bytes;
    int entry_errno;

    entry_errno = ft_errno;
    outstanding_bytes = g_cma_leak_outstanding_bytes;
    ft_errno = entry_errno;
    return (outstanding_bytes);
}

ft_string cma_leak_detection_report(bool clear_after)
{
    ft_string report;
    ft_string allocation_count_string;
    ft_string allocation_bytes_string;
    size_t record_count;
    size_t record_index;
    bool was_enabled;
    bool error_state;
    int entry_errno;

    entry_errno = ft_errno;
    was_enabled = g_cma_leak_detection_enabled;
    error_state = g_cma_leak_detection_error;
    g_cma_leak_detection_enabled = false;
    report = ft_string();
    if (report.get_error() != ER_SUCCESS)
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "cma leak detection status:\n"))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "active: "))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (was_enabled && !error_state)
    {
        if (!cma_leak_report_append(report, "true\n"))
        {
            cma_leak_tracker_resume(was_enabled, error_state);
            ft_errno = report.get_error();
            return (report);
        }
    }
    else
    {
        if (!cma_leak_report_append(report, "false\n"))
        {
            cma_leak_tracker_resume(was_enabled, error_state);
            ft_errno = report.get_error();
            return (report);
        }
    }
    allocation_count_string = ft_to_string(g_cma_leak_records.size());
    if (allocation_count_string.get_error() != ER_SUCCESS)
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = allocation_count_string.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "outstanding allocations: "))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append_string(report, allocation_count_string))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "\n"))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    allocation_bytes_string = ft_to_string(g_cma_leak_outstanding_bytes);
    if (allocation_bytes_string.get_error() != ER_SUCCESS)
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = allocation_bytes_string.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "outstanding bytes: "))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append_string(report, allocation_bytes_string))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    if (!cma_leak_report_append(report, "\n"))
    {
        cma_leak_tracker_resume(was_enabled, error_state);
        ft_errno = report.get_error();
        return (report);
    }
    record_count = g_cma_leak_records.size();
    if (record_count != 0)
    {
        if (!cma_leak_report_append(report, "allocations:\n"))
        {
            cma_leak_tracker_resume(was_enabled, error_state);
            ft_errno = report.get_error();
            return (report);
        }
        record_index = 0;
        while (record_index < record_count)
        {
            s_cma_leak_record record;
            ft_string index_string;
            ft_string size_string;

            record = g_cma_leak_records[record_index];
            if (g_cma_leak_records.get_error() != ER_SUCCESS)
            {
                cma_leak_tracker_handle_error(g_cma_leak_records.get_error());
                cma_leak_tracker_resume(false, true);
                return (report);
            }
            index_string = ft_to_string(record_index);
            if (index_string.get_error() != ER_SUCCESS)
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = index_string.get_error();
                return (report);
            }
            size_string = ft_to_string(static_cast<unsigned long>(record.size));
            if (size_string.get_error() != ER_SUCCESS)
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = size_string.get_error();
                return (report);
            }
            if (!cma_leak_report_append(report, "  ["))
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = report.get_error();
                return (report);
            }
            if (!cma_leak_report_append_string(report, index_string))
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = report.get_error();
                return (report);
            }
            if (!cma_leak_report_append(report, "] "))
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = report.get_error();
                return (report);
            }
            if (!cma_leak_report_append_string(report, size_string))
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = report.get_error();
                return (report);
            }
            if (!cma_leak_report_append(report, " bytes\n"))
            {
                cma_leak_tracker_resume(was_enabled, error_state);
                ft_errno = report.get_error();
                return (report);
            }
            record_index += 1;
        }
    }
    if (clear_after)
    {
        if (!cma_leak_tracker_clear_records(true))
            return (report);
        if (was_enabled)
            g_cma_leak_detection_enabled = true;
        else
            g_cma_leak_detection_enabled = false;
    }
    else
        cma_leak_tracker_resume(was_enabled, error_state);
    ft_errno = entry_errno;
    return (report);
}
