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
static ft_bool                                     g_trace_session_active = FT_FALSE;
static ft_bool                                     g_trace_first_event = FT_TRUE;
static std::chrono::steady_clock::time_point    g_trace_session_start;
static std::vector<t_time_trace_frame>          g_trace_stack;

static ft_bool time_trace_write_raw(const char *buffer, ft_size_t length, ft_bool flush)
{
    ft_size_t  written;

    if (buffer == ft_nullptr && length != 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    written = std::fwrite(buffer, 1, length, g_trace_file);
    if (written != length)
    {
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    if (flush)
    {
        if (std::fflush(g_trace_file) != 0)
        {
            (void)(FT_ERR_IO);
            return (FT_FALSE);
        }
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

static std::string  time_trace_escape_json(const char *input)
{
    std::string escaped;
    ft_size_t      index;

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

static int64_t    time_trace_elapsed_microseconds(std::chrono::steady_clock::time_point moment)
{
    std::chrono::steady_clock::duration         duration_since_start;
    int64_t                                   microseconds;

    duration_since_start = moment - g_trace_session_start;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_start).count();
    return (microseconds);
}

static std::string  time_trace_long_long_to_string(int64_t value)
{
    return (std::to_string(value));
}

static ft_bool time_trace_write_event_line(const std::string &line)
{
    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    if (!g_trace_first_event)
    {
        if (!time_trace_write_raw(",\n", 2, FT_FALSE))
            return (FT_FALSE);
    }
    if (!time_trace_write_raw(line.c_str(), line.size(), FT_TRUE))
        return (FT_FALSE);
    if (g_trace_first_event)
        g_trace_first_event = FT_FALSE;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

static ft_bool time_trace_write_duration_event(const std::string &name,
    const std::string &category, int64_t start_us, int64_t duration_us)
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

static ft_bool time_trace_write_instant_event(const std::string &name,
    const std::string &category, int64_t timestamp_us)
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

ft_bool    time_trace_begin_session(const char *file_path)
{
    const char  *header;
    ft_size_t      header_length;

    if (g_trace_session_active)
    {
        (void)(FT_ERR_ALREADY_INITIALISED);
        return (FT_FALSE);
    }
    if (file_path == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    g_trace_file = std::fopen(file_path, "w");
    if (g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    header = "{\"traceEvents\":[\n";
    header_length = std::strlen(header);
    if (std::fwrite(header, 1, header_length, g_trace_file) != header_length)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    if (std::fflush(g_trace_file) != 0)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    g_trace_session_start = std::chrono::steady_clock::now();
    g_trace_session_active = FT_TRUE;
    g_trace_first_event = FT_TRUE;
    g_trace_stack.clear();
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_trace_end_session(void)
{
    const char  *footer;
    ft_size_t      footer_length;

    if (!g_trace_session_active)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    if (!g_trace_stack.empty())
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    footer = "\n]}\n";
    footer_length = std::strlen(footer);
    if (std::fwrite(footer, 1, footer_length, g_trace_file) != footer_length)
    {
        std::fclose(g_trace_file);
        g_trace_file = ft_nullptr;
        g_trace_session_active = FT_FALSE;
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    if (std::fclose(g_trace_file) != 0)
    {
        g_trace_file = ft_nullptr;
        g_trace_session_active = FT_FALSE;
        (void)(FT_ERR_IO);
        return (FT_FALSE);
    }
    g_trace_file = ft_nullptr;
    g_trace_session_active = FT_FALSE;
    g_trace_first_event = FT_TRUE;
    g_trace_stack.clear();
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_trace_begin_event(const char *name, const char *category)
{
    t_time_trace_frame frame;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
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
        (void)(FT_ERR_NO_MEMORY);
        return (FT_FALSE);
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_trace_end_event(void)
{
    std::chrono::steady_clock::time_point    end_time;
    t_time_trace_frame                        frame;
    int64_t                                start_us;
    int64_t                                duration_us;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    if (g_trace_stack.empty())
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
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
        return (FT_FALSE);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_trace_instant_event(const char *name, const char *category)
{
    std::chrono::steady_clock::time_point    now;
    std::string                              event_name;
    std::string                              event_category;
    int64_t                                timestamp_us;

    if (!g_trace_session_active || g_trace_file == ft_nullptr)
    {
        (void)(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
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
        return (FT_FALSE);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
