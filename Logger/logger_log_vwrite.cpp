#include "logger_internal.hpp"
#include <cerrno>
#include <unistd.h>
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"

static bool logger_sink_equals(const s_log_sink &left, const s_log_sink &right)
{
    if (left.function != right.function)
        return (false);
    if (left.user_data != right.user_data)
        return (false);
    return (true);
}

static bool logger_sink_snapshot_contains(const ft_vector<s_log_sink> &snapshot, const s_log_sink &entry, int &error_code)
{
    size_t snapshot_count;
    size_t snapshot_index;

    snapshot_count = snapshot.size();
    error_code = snapshot.get_error();
    if (error_code != FT_ERR_SUCCESSS)
        return (false);
    snapshot_index = 0;
    while (snapshot_index < snapshot_count)
    {
        s_log_sink snapshot_entry;

        snapshot_entry = snapshot[snapshot_index];
        error_code = snapshot.get_error();
        if (error_code != FT_ERR_SUCCESSS)
            return (false);
        if (logger_sink_equals(snapshot_entry, entry))
            return (true);
        snapshot_index += 1;
    }
    return (false);
}

static int logger_append_quoted_token(ft_string &buffer, const char *value)
{
    size_t index;
    static const char hex_digits[] = "0123456789ABCDEF";

    if (!value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    buffer.append('"');
    if (buffer.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = buffer.get_error();
        return (-1);
    }
    index = 0;
    while (value[index] != '\0')
    {
        char character;

        character = value[index];
        if (character == '"' || character == '\\')
        {
            buffer.append('\\');
            if (buffer.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = buffer.get_error();
                return (-1);
            }
            buffer.append(character);
            if (buffer.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = buffer.get_error();
                return (-1);
            }
        }
        else if (static_cast<unsigned char>(character) < 0x20)
        {
            char escape_buffer[5];

            escape_buffer[0] = '\\';
            escape_buffer[1] = 'x';
            escape_buffer[2] = hex_digits[(static_cast<unsigned char>(character) >> 4) & 0x0F];
            escape_buffer[3] = hex_digits[static_cast<unsigned char>(character) & 0x0F];
            escape_buffer[4] = '\0';
            buffer.append(escape_buffer);
            if (buffer.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = buffer.get_error();
                return (-1);
            }
        }
        else
        {
            buffer.append(character);
            if (buffer.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = buffer.get_error();
                return (-1);
            }
        }
        index += 1;
    }
    buffer.append('"');
    if (buffer.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = buffer.get_error();
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

int logger_build_standard_message(t_log_level level, const ft_string &message_text,
    const ft_string &context_fragment, ft_string &formatted_message)
{
    ft_string timestamp;
    ft_string assembled;
    int severity_value;
    char severity_buffer[16];
    int severity_length;

    if (message_text.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = message_text.get_error();
        return (-1);
    }
    timestamp = time_format_iso8601(time_now());
    if (timestamp.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = timestamp.get_error();
        return (-1);
    }
    severity_value = ft_log_level_to_severity(level);
    severity_length = pf_snprintf(severity_buffer, sizeof(severity_buffer), "%d", severity_value);
    if (severity_length <= 0 || severity_length >= static_cast<int>(sizeof(severity_buffer)))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    assembled = ft_string("time=");
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(timestamp);
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(" level=");
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(ft_level_to_str(level));
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(" severity=");
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(severity_buffer);
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    assembled.append(" message=");
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    if (logger_append_quoted_token(assembled, message_text.c_str()) != 0)
        return (-1);
    if (context_fragment.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = context_fragment.get_error();
        return (-1);
    }
    if (context_fragment.size() > 0)
    {
        assembled.append(' ');
        if (assembled.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = assembled.get_error();
            return (-1);
        }
        assembled.append(context_fragment);
        if (assembled.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = assembled.get_error();
            return (-1);
        }
    }
    assembled.append('\n');
    if (assembled.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = assembled.get_error();
        return (-1);
    }
    formatted_message = assembled;
    if (formatted_message.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = formatted_message.get_error();
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void ft_log_vwrite(t_log_level level, const char *fmt, va_list args)
{
    ft_vector<s_log_sink> sinks_snapshot;
    ft_vector<s_redaction_rule> redaction_snapshot;
    ft_string message_text;
    ft_string context_fragment;
    ft_string final_message;
    size_t sink_count;
    bool use_color;
    int final_error;
    int format_error;
    char message_buffer[1024];
    va_list args_copy;

    if (!fmt)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (level < g_level)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    va_copy(args_copy, args);
    int formatted_length = pf_vsnprintf(message_buffer, sizeof(message_buffer), fmt, args_copy);
    va_end(args_copy);
    format_error = ft_global_error_stack_pop_newest();
    if (formatted_length < 0)
    {
        if (format_error != FT_ERR_SUCCESSS)
            ft_errno = format_error;
        return ;
    }
    message_text = ft_string(message_buffer);
    if (message_text.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = message_text.get_error();
        return ;
    }
    if (logger_lock_sinks() != 0)
        return ;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return ;
        ft_errno = final_error;
        return ;
    }
    use_color = false;
    if (g_use_color && sink_count == 0 && isatty(1))
        use_color = true;
    if (sink_count != 0)
    {
        size_t index;

        index = 0;
        while (index < sink_count)
        {
            s_log_sink entry;

            entry = g_sinks[index];
            if (g_sinks.get_error() != FT_ERR_SUCCESSS)
            {
                final_error = g_sinks.get_error();
                if (logger_unlock_sinks() != 0)
                    return ;
                ft_errno = final_error;
                return ;
            }
            int contains_error;
            bool is_duplicate;

            contains_error = FT_ERR_SUCCESSS;
            is_duplicate = logger_sink_snapshot_contains(sinks_snapshot, entry, contains_error);
            if (contains_error != FT_ERR_SUCCESSS)
            {
                final_error = contains_error;
                if (logger_unlock_sinks() != 0)
                    return ;
                ft_errno = final_error;
                return ;
            }
            if (!is_duplicate)
            {
                sinks_snapshot.push_back(entry);
                if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
                {
                    final_error = sinks_snapshot.get_error();
                    if (logger_unlock_sinks() != 0)
                        return ;
                    ft_errno = final_error;
                    return ;
                }
            }
            index += 1;
        }
    }
    if (logger_copy_redaction_rules(redaction_snapshot) != 0)
    {
        final_error = ft_errno;
        if (logger_unlock_sinks() != 0)
            return ;
        ft_errno = final_error;
        return ;
    }
    if (logger_unlock_sinks() != 0)
        return ;
    if (logger_apply_redactions(message_text, redaction_snapshot) != 0)
        return ;
    if (logger_context_format_flat(context_fragment) != 0)
        return ;
    if (context_fragment.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = context_fragment.get_error();
        return ;
    }
    if (context_fragment.size() > 0)
    {
        if (logger_apply_redactions(context_fragment, redaction_snapshot) != 0)
            return ;
    }
    if (logger_build_standard_message(level, message_text, context_fragment, final_message) != 0)
        return ;
    if (final_message.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = final_message.get_error();
        return ;
    }
    if (sink_count == 0)
    {
        ssize_t write_result;

        if (use_color)
        {
            const char *color_code;

            color_code = "";
            if (level == LOG_LEVEL_DEBUG)
                color_code = "\x1b[36m";
            else if (level == LOG_LEVEL_INFO)
                color_code = "\x1b[32m";
            else if (level == LOG_LEVEL_WARN)
                color_code = "\x1b[33m";
            else
                color_code = "\x1b[31m";
            write_result = write(1, color_code, ft_strlen(color_code));
            if (write_result < 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
            write_result = write(1, final_message.c_str(), final_message.size());
            if (write_result < 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
            write_result = write(1, "\x1b[0m", 4);
            if (write_result < 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
        }
        else
        {
            write_result = write(1, final_message.c_str(), final_message.size());
            if (write_result < 0)
            {
                ft_errno = ft_map_system_error(errno);
                return ;
            }
        }
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    size_t snapshot_count;
    size_t index;

    snapshot_count = sinks_snapshot.size();
    if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = sinks_snapshot.get_error();
        return ;
    }
    index = 0;
    while (index < snapshot_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[index];
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = sinks_snapshot.get_error();
            return ;
        }
        bool sink_lock_acquired;
        int sink_error;
        int sink_lock_error;

        sink_lock_acquired = false;
        sink_error = FT_ERR_SUCCESSS;
        sink_lock_error = log_sink_lock(&entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESSS)
            sink_error = sink_lock_error;
        else
        {
            bool rotate_for_size_pre;
            bool rotate_for_age_pre;
            s_file_sink *file_sink;

            rotate_for_size_pre = false;
            rotate_for_age_pre = false;
            file_sink = ft_nullptr;
            if (entry.function == ft_file_sink)
            {
                file_sink = static_cast<s_file_sink *>(entry.user_data);
                if (logger_prepare_rotation(file_sink, &rotate_for_size_pre, &rotate_for_age_pre) != 0)
                    sink_error = ft_errno;
            }
            if (sink_error == FT_ERR_SUCCESSS)
            {
                entry.function(final_message.c_str(), entry.user_data);
                if (ft_errno != FT_ERR_SUCCESSS)
                    sink_error = ft_errno;
            }
            if (sink_error == FT_ERR_SUCCESSS && entry.function == ft_file_sink)
            {
                bool rotate_for_size_post;
                bool rotate_for_age_post;

                rotate_for_size_post = false;
                rotate_for_age_post = false;
                if (logger_prepare_rotation(file_sink, &rotate_for_size_post, &rotate_for_age_post) != 0)
                    sink_error = ft_errno;
                else if (rotate_for_size_pre || rotate_for_age_pre || rotate_for_size_post || rotate_for_age_post)
                {
                    logger_execute_rotation(file_sink);
                    if (ft_errno != FT_ERR_SUCCESSS)
                        sink_error = ft_errno;
                }
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&entry, sink_lock_acquired);
        if (sink_error != FT_ERR_SUCCESSS)
        {
            ft_errno = sink_error;
            return ;
        }
        index += 1;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
