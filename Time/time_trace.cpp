#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>
#include <chrono>
#include <string>
#include <vector>
#include <new>
#include <cstring>
#include <cstddef>

typedef struct s_time_trace_frame
{
    std::string                                 name;
    std::string                                 category;
    std::chrono::steady_clock::time_point       start_time;
}   t_time_trace_frame;

static std::FILE                                *g_trace_file = ft_nullptr;
static bool                                     g_trace_session_active = false;
static bool                                     g_trace_first_event = true;
static std::chrono::steady_clock::time_point    g_trace_session_start;
static std::vector<t_time_trace_frame>          g_trace_stack;

static bool time_trace_write_raw(const char *buffer, size_t length, bool flush)
{
    size_t  written;

    if (buffer == ft_nullptr && length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    written = std::fwrite(buffer, 1, length, g_trace_file);
    if (written != length)
    {
        ft_errno = FT_ERR_IO;
        return (false);
    }
    if (flush)
    {
        if (std::fflush(g_trace_file) != 0)
        {
            ft_errno = FT_ERR_IO;
            return (false);
        }
    }
    return (true);
}

static std::string  time_trace_escape_json(const char *input)
{
    std::string escaped;
    size_t      index;

    if (input == ft_nullptr)
        return (std::string());
    index = 0;
    while (input[index] != '\0')
    {
        char character;

        character = input[index];
        if (character == '"' || character == '\\')
        {
            escaped.push_back('\\');
            escaped.push_back(character);
        }
        else if (character == '\n')
        {
            escaped.push_back('\\');
            escaped.push_back('n');
        }
        else if (character == '\r')
        {
            escaped.push_back('\\');
            escaped.push_back('r');
        }
        else if (character == '\t')
        {
            escaped.push_back('\\');
            escaped.push_back('t');
        }
        else
            escaped.push_back(character);
        index++;
    }
    return (escaped);
}

static long long    time_trace_elapsed_microseconds(std::chrono::steady_clock::time_point moment)
{
    std::chrono::steady_clock::duration         duration_since_start;
    long long                                   microseconds;

    duration_since_start = moment - g_trace_session_start;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_start).count();
    return (microseconds);
}

static std::string  time_trace_long_long_to_string(long long value)
{
    return (std::to_string(value));
}

static bool time_trace_write_event_line(const std::string &line)
{
    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    if (!g_trace_first_event)
    {
        if (!time_trace_write_raw(",\n", 2, false))
            return (false);
    }
    if (!time_trace_write_raw(line.c_str(), line.size(), true))
        return (false);
    if (g_trace_first_event)
        g_trace_first_event = false;
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}

static bool time_trace_write_duration_event(const std::string &name,
    const std::string &category, long long start_us, long long duration_us)
{
    std::string line;
    std::string escaped_name;
    std::string escaped_category;

    escaped_name = time_trace_escape_json(name.c_str());
    escaped_category = time_trace_escape_json(category.c_str());
    line = "{\"name\":\"";
    line += escaped_name;
    line += "\",\"cat\":\"";
    line += escaped_category;
    line += "\",\"ph\":\"X\",\"ts\":";
    line += time_trace_long_long_to_string(start_us);
    line += ",\"dur\":";
    line += time_trace_long_long_to_string(duration_us);
    line += ",\"pid\":0,\"tid\":0}";
    line += '\n';
    return (time_trace_write_event_line(line));
}

static bool time_trace_write_instant_event(const std::string &name,
    const std::string &category, long long timestamp_us)
{
    std::string line;
    std::string escaped_name;
    std::string escaped_category;

    escaped_name = time_trace_escape_json(name.c_str());
    escaped_category = time_trace_escape_json(category.c_str());
    line = "{\"name\":\"";
    line += escaped_name;
    line += "\",\"cat\":\"";
    line += escaped_category;
    line += "\",\"ph\":\"i\",\"ts\":";
    line += time_trace_long_long_to_string(timestamp_us);
    line += ",\"s\":\"t\",\"pid\":0,\"tid\":0}";
    line += '\n';
    return (time_trace_write_event_line(line));
}

bool    time_trace_begin_session(const char *file_path)
{
    const char  *header;
    size_t      header_length;

    if (g_trace_session_active)
    {
        ft_errno = FT_ERR_ALREADY_INITIALIZED;
        return (false);
    }
    if (file_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    g_trace_file = std::fopen(file_path, "w");
    if (g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_IO;
        return (false);
    }
    header = "{\"traceEvents\":[\n";
    header_length = std::strlen(header);
    if (std::fwrite(header, 1, header_length, g_trace_file) != header_length)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        ft_errno = FT_ERR_IO;
        return (false);
    }
    if (std::fflush(g_trace_file) != 0)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        ft_errno = FT_ERR_IO;
        return (false);
    }
    g_trace_session_start = std::chrono::steady_clock::now();
    g_trace_session_active = true;
    g_trace_first_event = true;
    g_trace_stack.clear();
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}

bool    time_trace_end_session(void)
{
    const char  *footer;
    size_t      footer_length;

    if (!g_trace_session_active)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    if (!g_trace_stack.empty())
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    footer = "\n]}\n";
    footer_length = std::strlen(footer);
    if (std::fwrite(footer, 1, footer_length, g_trace_file) != footer_length)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        g_trace_session_active = false;
        ft_errno = FT_ERR_IO;
        return (false);
    }
    if (std::fclose(g_trace_file) != 0)
    {
        g_trace_file = ft_nullptr;
        g_trace_session_active = false;
        ft_errno = FT_ERR_IO;
        return (false);
    }
    g_trace_file = ft_nullptr;
    g_trace_session_active = false;
    g_trace_first_event = true;
    g_trace_stack.clear();
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}

bool    time_trace_begin_event(const char *name, const char *category)
{
    t_time_trace_frame frame;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    if (name == ft_nullptr)
        frame.name = "";
    else
        frame.name = name;
    if (category == ft_nullptr)
        frame.category = "";
    else
        frame.category = category;
    frame.start_time = std::chrono::steady_clock::now();
    try
    {
        g_trace_stack.push_back(frame);
    }
    catch (const std::bad_alloc &)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (false);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}

bool    time_trace_end_event(void)
{
    std::chrono::steady_clock::time_point    end_time;
    t_time_trace_frame                        frame;
    long long                                start_us;
    long long                                duration_us;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    if (g_trace_stack.empty())
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    frame = g_trace_stack.back();
    g_trace_stack.pop_back();
    end_time = std::chrono::steady_clock::now();
    start_us = time_trace_elapsed_microseconds(frame.start_time);
    duration_us = time_trace_elapsed_microseconds(end_time) - start_us;
    if (duration_us < 0)
        duration_us = 0;
    if (!time_trace_write_duration_event(frame.name, frame.category,
            start_us, duration_us))
        return (false);
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}

bool    time_trace_instant_event(const char *name, const char *category)
{
    std::chrono::steady_clock::time_point    now;
    std::string                              event_name;
    std::string                              event_category;
    long long                                timestamp_us;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    if (name == ft_nullptr)
        event_name = "";
    else
        event_name = name;
    if (category == ft_nullptr)
        event_category = "";
    else
        event_category = category;
    now = std::chrono::steady_clock::now();
    timestamp_us = time_trace_elapsed_microseconds(now);
    if (!time_trace_write_instant_event(event_name, event_category,
            timestamp_us))
        return (false);
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}
