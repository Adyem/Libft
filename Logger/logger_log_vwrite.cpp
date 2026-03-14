#include "logger_internal.hpp"
#include <unistd.h>
#include "../Time/time.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"

static int32_t logger_append_quoted_token(ft_string &buffer, const char *value)
{
    ft_size_t entry_index;
    static const char hex_digits[] = "0123456789ABCDEF";

    if (!value)
        return (FT_ERR_INTERNAL);
    buffer.append('"');
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    entry_index = 0;
    while (value[entry_index] != '\0')
    {
        unsigned char character;

        character = static_cast<unsigned char>(value[entry_index]);
        if (character == '"' || character == '\\')
        {
            buffer.append('\\');
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (FT_ERR_INTERNAL);
            buffer.append(static_cast<char>(character));
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (FT_ERR_INTERNAL);
        }
        else if (character < 0x20)
        {
            char escape_buffer[5];

            escape_buffer[0] = '\\';
            escape_buffer[1] = 'x';
            escape_buffer[2] = hex_digits[(character >> 4) & 0x0F];
            escape_buffer[3] = hex_digits[character & 0x0F];
            escape_buffer[4] = '\0';
            buffer.append(escape_buffer);
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (FT_ERR_INTERNAL);
        }
        else
        {
            buffer.append(static_cast<char>(character));
            if (ft_string::get_error() != FT_ERR_SUCCESS)
                return (FT_ERR_INTERNAL);
        }
        entry_index += 1;
    }
    buffer.append('"');
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

int32_t logger_build_standard_message(t_log_level level, const ft_string &message_text,
    const ft_string &context_fragment, ft_string &formatted_message)
{
    ft_string timestamp;
    ft_string assembled;
    int32_t severity_value;
    char severity_buffer[16];
    int32_t severity_length;

    timestamp = time_format_iso8601(time_now());
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    severity_value = ft_log_level_to_severity(level);
    severity_length = pf_snprintf(severity_buffer, sizeof(severity_buffer), "%d",
            severity_value);
    if (severity_length <= 0
        || severity_length >= static_cast<int32_t>(sizeof(severity_buffer)))
        return (FT_ERR_INTERNAL);
    if (assembled.initialize("time=") != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(timestamp);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(" level=");
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(ft_level_to_str(level));
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(" severity=");
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(severity_buffer);
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    assembled.append(" message=");
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (logger_append_quoted_token(assembled, message_text.c_str()) != 0)
        return (FT_ERR_INTERNAL);
    if (context_fragment.size() > 0)
    {
        assembled.append(' ');
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        assembled.append(context_fragment);
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    assembled.append('\n');
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    formatted_message = assembled;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

void ft_log_vwrite(t_log_level level, const char *format_string, va_list argument_list)
{
    ft_vector<s_redaction_rule> redaction_snapshot;
    ft_string message_text;
    ft_string context_fragment;
    ft_string final_message;
    ft_size_t sink_count;
    ft_bool use_color;
    char message_buffer[1024];
    va_list args_copy;
    int32_t formatted_length;
    ft_size_t entry_index;

    if (!format_string)
        return ;
    if (level < g_level)
        return ;
    va_copy(args_copy, argument_list);
    formatted_length = pf_vsnprintf(message_buffer, sizeof(message_buffer), format_string,
            args_copy);
    va_end(args_copy);
    if (formatted_length < 0)
        return ;
    if (message_text.initialize(message_buffer) != FT_ERR_SUCCESS)
        return ;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return ;
    if (logger_lock_sinks() != 0)
        return ;
    sink_count = g_sinks.size();
    use_color = FT_FALSE;
    if (g_use_color && sink_count == 0 && isatty(1))
        use_color = FT_TRUE;
    if (logger_copy_redaction_rules(redaction_snapshot) != 0)
    {
        (void)logger_unlock_sinks();
        return ;
    }
    if (logger_unlock_sinks() != 0)
        return ;
    if (logger_apply_redactions(message_text, redaction_snapshot) != 0)
        return ;
    if (logger_context_format_flat(context_fragment) != 0)
        return ;
    if (context_fragment.size() > 0)
    {
        if (logger_apply_redactions(context_fragment, redaction_snapshot) != 0)
            return ;
    }
    if (logger_build_standard_message(level, message_text, context_fragment,
            final_message) != 0)
        return ;
    if (sink_count == 0)
    {
        if (use_color)
        {
            const char *color_code;

            color_code = "\x1b[31m";
            if (level == LOG_LEVEL_DEBUG)
                color_code = "\x1b[36m";
            else if (level == LOG_LEVEL_INFO)
                color_code = "\x1b[32m";
            else if (level == LOG_LEVEL_WARN)
                color_code = "\x1b[33m";
            (void)write(1, color_code, ft_strlen(color_code));
            (void)write(1, final_message.c_str(), final_message.size());
            (void)write(1, "\x1b[0m", 4);
            return ;
        }
        (void)write(1, final_message.c_str(), final_message.size());
        return ;
    }
    entry_index = 0;
    if (logger_lock_sinks() != 0)
        return ;
    while (entry_index < sink_count && entry_index < g_sinks.size())
    {
        s_log_sink entry;
        ft_bool sink_lock_acquired;

        entry = g_sinks[entry_index];
        sink_lock_acquired = FT_FALSE;
        if (entry.function != ft_nullptr
            && log_sink_lock(&entry, &sink_lock_acquired) == FT_ERR_SUCCESS)
        {
            entry.function(final_message.c_str(), entry.user_data);
            if (entry.function == ft_file_sink)
                logger_execute_rotation(static_cast<s_file_sink *>(entry.user_data));
        }
        if (sink_lock_acquired)
            log_sink_unlock(&entry, sink_lock_acquired);
        entry_index += 1;
    }
    (void)logger_unlock_sinks();
    return ;
}
