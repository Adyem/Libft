#include "logger_internal.hpp"
#include <unistd.h>
#include <cerrno>
#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"

struct s_json_sink_field
{
    ft_string key;
    ft_string value;
    bool has_value;
    bool value_is_json;

    s_json_sink_field()
        : key(), value(), has_value(false), value_is_json(false)
    {
        return ;
    }
};

static void logger_json_sink_append_literal(ft_string &buffer, const char *literal, int &error_code)
{
    if (error_code != FT_ERR_SUCCESSS)
        return ;
    if (!literal)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    buffer.append(literal);
    if (buffer.get_error() != FT_ERR_SUCCESSS)
        error_code = buffer.get_error();
    return ;
}

static void logger_json_sink_append_character_sequence(ft_string &buffer, const char *sequence, int &error_code)
{
    if (error_code != FT_ERR_SUCCESSS)
        return ;
    if (!sequence)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    buffer.append(sequence);
    if (buffer.get_error() != FT_ERR_SUCCESSS)
        error_code = buffer.get_error();
    return ;
}

static void logger_json_sink_append_json_escaped(ft_string &buffer, char character, int &error_code)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    if (error_code != FT_ERR_SUCCESSS)
        return ;
    if (character == '\\' || character == '"')
    {
        buffer.append('\\');
        if (buffer.get_error() != FT_ERR_SUCCESSS)
        {
            error_code = buffer.get_error();
            return ;
        }
        buffer.append(character);
        if (buffer.get_error() != FT_ERR_SUCCESSS)
            error_code = buffer.get_error();
        return ;
    }
    if (character >= 0 && character < 0x20)
    {
        char escape_buffer[7];

        escape_buffer[0] = '\\';
        escape_buffer[1] = 'u';
        escape_buffer[2] = '0';
        escape_buffer[3] = '0';
        escape_buffer[4] = hex_digits[(static_cast<unsigned char>(character) >> 4) & 0x0F];
        escape_buffer[5] = hex_digits[static_cast<unsigned char>(character) & 0x0F];
        escape_buffer[6] = '\0';
        logger_json_sink_append_literal(buffer, escape_buffer, error_code);
        return ;
    }
    buffer.append(character);
    if (buffer.get_error() != FT_ERR_SUCCESSS)
        error_code = buffer.get_error();
    return ;
}

static void logger_json_sink_append_json_string(ft_string &buffer, const char *value, int &error_code)
{
    size_t index;

    if (error_code != FT_ERR_SUCCESSS)
        return ;
    if (!value)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    buffer.append('"');
    if (buffer.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = buffer.get_error();
        return ;
    }
    index = 0;
    while (value[index] != '\0' && error_code == FT_ERR_SUCCESSS)
    {
        logger_json_sink_append_json_escaped(buffer, value[index], error_code);
        index += 1;
    }
    if (error_code != FT_ERR_SUCCESSS)
        return ;
    buffer.append('"');
    if (buffer.get_error() != FT_ERR_SUCCESSS)
        error_code = buffer.get_error();
    return ;
}

static int logger_json_sink_hex_value(char character)
{
    if (character >= '0' && character <= '9')
        return (character - '0');
    if (character >= 'a' && character <= 'f')
        return (10 + character - 'a');
    if (character >= 'A' && character <= 'F')
        return (10 + character - 'A');
    return (-1);
}

static void logger_json_sink_decode_quoted(const char *message, size_t &index, char *destination, size_t capacity)
{
    size_t dest_index;

    dest_index = 0;
    while (message[index] != '\0' && message[index] != '"')
    {
        char character;

        character = message[index];
        if (character == '\\' && message[index + 1] != '\0')
        {
            index += 1;
            character = message[index];
            if (character == 'x' && message[index + 1] != '\0' && message[index + 2] != '\0')
            {
                int high_value;
                int low_value;

                high_value = logger_json_sink_hex_value(message[index + 1]);
                low_value = logger_json_sink_hex_value(message[index + 2]);
                if (high_value >= 0 && low_value >= 0)
                {
                    character = static_cast<char>((high_value << 4) | low_value);
                    index += 2;
                }
            }
        }
        if (dest_index + 1 < capacity)
        {
            destination[dest_index] = character;
            dest_index += 1;
        }
        index += 1;
    }
    destination[dest_index] = '\0';
    if (message[index] == '"')
        index += 1;
    return ;
}

static void logger_json_sink_decode_unquoted(const char *message, size_t &index, char *destination, size_t capacity)
{
    size_t dest_index;

    dest_index = 0;
    while (message[index] != '\0' && message[index] != ' ' && message[index] != '\n')
    {
        if (dest_index + 1 < capacity)
        {
            destination[dest_index] = message[index];
            dest_index += 1;
        }
        index += 1;
    }
    destination[dest_index] = '\0';
    return ;
}

int ft_log_add_sink(t_log_sink sink, void *user_data)
{
    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    s_log_sink entry;
    int final_error;

    if (log_sink_prepare_thread_safety(&entry) != 0)
        return (-1);
    if (logger_lock_sinks() != 0)
    {
        log_sink_teardown_thread_safety(&entry);
        return (-1);
    }
    entry.function = sink;
    entry.user_data = user_data;
    g_sinks.push_back(entry);
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        log_sink_teardown_thread_safety(&entry);
        if (logger_unlock_sinks() != 0)
        {
            return (-1);
        }
        ft_errno = final_error;
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    if (logger_unlock_sinks() != 0)
    {
        return (-1);
    }
    ft_errno = final_error;
    return (0);
}

void ft_json_sink(const char *message, void *user_data)
{
    char time_buffer[64];
    char level_buffer[16];
    char severity_buffer[16];
    char key_buffer[128];
    char value_buffer[1024];
    char message_buffer[2048];
    ft_vector<s_json_sink_field> context_fields;
    size_t index;
    int fd;
    bool message_is_json;
    int error_code;

    if (!message)
        return ;
    time_buffer[0] = '\0';
    level_buffer[0] = '\0';
    severity_buffer[0] = '\0';
    message_buffer[0] = '\0';
    message_is_json = false;
    fd = 1;
    if (user_data)
        fd = *static_cast<int *>(user_data);
    index = 0;
    while (message[index] == ' ')
        index += 1;
    while (message[index] != '\0' && message[index] != '\n')
    {
        size_t key_length;
        bool has_value;
        bool value_is_json;

        key_length = 0;
        while (message[index] != '\0' && message[index] != '=' && message[index] != ' ' && message[index] != '\n')
        {
            if (key_length + 1 < sizeof(key_buffer))
            {
                key_buffer[key_length] = message[index];
                key_length += 1;
            }
            index += 1;
        }
        key_buffer[key_length] = '\0';
        while (message[index] == ' ')
            index += 1;
        has_value = false;
        value_is_json = false;
        value_buffer[0] = '\0';
        if (message[index] == '=')
        {
            has_value = true;
            index += 1;
            if (message[index] == '"')
            {
                index += 1;
                logger_json_sink_decode_quoted(message, index, value_buffer, sizeof(value_buffer));
            }
            else
                logger_json_sink_decode_unquoted(message, index, value_buffer, sizeof(value_buffer));
            if (value_buffer[0] == '{' || value_buffer[0] == '[')
                value_is_json = true;
        }
        while (message[index] == ' ')
            index += 1;
        if (key_buffer[0] == '\0')
        {
            if (message[index] == '\0' || message[index] == '\n')
                break ;
            continue ;
        }
        if (ft_strcmp(key_buffer, "time") == 0 && has_value)
            ft_strlcpy(time_buffer, value_buffer, sizeof(time_buffer));
        else if (ft_strcmp(key_buffer, "level") == 0 && has_value)
            ft_strlcpy(level_buffer, value_buffer, sizeof(level_buffer));
        else if (ft_strcmp(key_buffer, "severity") == 0 && has_value)
            ft_strlcpy(severity_buffer, value_buffer, sizeof(severity_buffer));
        else if (ft_strcmp(key_buffer, "message") == 0 && has_value)
        {
            ft_strlcpy(message_buffer, value_buffer, sizeof(message_buffer));
            message_is_json = value_is_json;
        }
        else
        {
            s_json_sink_field field;

            field.key = key_buffer;
            if (field.key.get_error() != FT_ERR_SUCCESSS)
                return ;
            field.has_value = has_value;
            field.value_is_json = value_is_json;
            if (has_value)
            {
                field.value = value_buffer;
                if (field.value.get_error() != FT_ERR_SUCCESSS)
                    return ;
            }
            context_fields.push_back(field);
            if (context_fields.get_error() != FT_ERR_SUCCESSS)
                return ;
        }
        if (message[index] == '\0' || message[index] == '\n')
            break ;
    }
    ft_string payload("{");
    error_code = payload.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_errno = error_code;
        return ;
    }
    logger_json_sink_append_literal(payload, "\"time\":", error_code);
    logger_json_sink_append_json_string(payload, time_buffer, error_code);
    logger_json_sink_append_literal(payload, ",\"level\":", error_code);
    logger_json_sink_append_json_string(payload, level_buffer, error_code);
    if (severity_buffer[0] != '\0')
    {
        logger_json_sink_append_literal(payload, ",\"severity\":", error_code);
        logger_json_sink_append_character_sequence(payload, severity_buffer, error_code);
    }
    logger_json_sink_append_literal(payload, ",\"message\":", error_code);
    if (message_is_json)
        logger_json_sink_append_character_sequence(payload, message_buffer, error_code);
    else
        logger_json_sink_append_json_string(payload, message_buffer, error_code);
    size_t context_count;
    size_t context_index;

    context_count = context_fields.size();
    if (context_fields.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = context_fields.get_error();
        return ;
    }
    context_index = 0;
    while (context_index < context_count && error_code == FT_ERR_SUCCESSS)
    {
        const s_json_sink_field &field = context_fields[context_index];

        logger_json_sink_append_literal(payload, ",\"", error_code);
        logger_json_sink_append_character_sequence(payload, field.key.c_str(), error_code);
        logger_json_sink_append_literal(payload, "\":", error_code);
        if (field.has_value)
        {
            if (field.value_is_json)
                logger_json_sink_append_character_sequence(payload, field.value.c_str(), error_code);
            else
                logger_json_sink_append_json_string(payload, field.value.c_str(), error_code);
        }
        else
            logger_json_sink_append_literal(payload, "true", error_code);
        context_index += 1;
    }
    logger_json_sink_append_literal(payload, "}\n", error_code);
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_errno = error_code;
        return ;
    }
    if (payload.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = payload.get_error();
        return ;
    }
    ssize_t write_result;

    write_result = write(fd, payload.c_str(), payload.size());
    (void)write_result;
    return ;
}
